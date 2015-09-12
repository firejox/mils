#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include <gbm.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

/*drm header*/
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "screen.h"
#include "common.h"

static const char *device_name = "/dev/dri/card0";

struct drm_fb {
    struct gbm_bo  *bo;
    uint32_t        fb_id;
    int             flip;
};

static struct _kms {
    int fd;

    drmModeConnector *con;
    drmModeEncoder   *enc;
    drmModeModeInfo  mode;
    drmModeCrtc     *save;

    drmEventContext evctx;

    struct drm_fb     *fb;
} kms;

static struct {
    struct gbm_device  *device;
    struct gbm_surface *surface;
} gbm;

static struct {
    EGLDisplay dpy;
    EGLContext ctx;
    EGLSurface surface;
    EGLConfig  config;

    cairo_device_t  *c_device;
    cairo_surface_t *c_surface;
        
} egl;

static struct {
    uint32_t width;
    uint32_t height;
} dpy_info;


static const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE,   1,
    EGL_GREEN_SIZE, 1,
    EGL_BLUE_SIZE,  1,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 1,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};


static void pageflip_handle (int fd, unsigned int frame,
        unsigned int sec, unsigned int usec, void *data) {
    struct gbm_bo *bo = (struct gbm_bo*) data;
    gbm_surface_release_buffer (gbm.surface, bo);
}

static void init_kms (void) {
    drmModeRes       *res;
    drmModeConnector *con;
    drmModeEncoder   *enc;
    int i;

    kms.fd = open (device_name, O_RDWR);
    if (kms.fd < 0) {
        fprintf (stderr, "failed to open %s!\n", device_name);
        exit (-1);
    }

    /*get drm mode resource from fd*/
    res = drmModeGetResources (kms.fd);
    if (res == NULL) {
        fprintf (stderr, "drmModeGetResources failed\n");
        exit(-1);
    }
    
    for (i = 0; i < res->count_connectors; i++) {
        con = drmModeGetConnector (kms.fd, res->connectors[i]);

        if (con == NULL)
            continue;

        if (con->connection == DRM_MODE_CONNECTED &&
                con->count_modes > 0)
            break;

        drmModeFreeConnector (con);
    }

    if (i == res->count_connectors) {
        fprintf (stderr, "No connected connector!\n");
        exit (-1);
    }

    for (i = 0; i < res->count_encoders; i++) {
        enc = drmModeGetEncoder (kms.fd, res->encoders[i]);

        if (enc == NULL)
            continue;

        if (enc->encoder_id == con->encoder_id)
            break;

        drmModeFreeEncoder (enc);
    }

    kms.con = con;
    kms.enc = enc;
    kms.mode = con->modes[0];
    kms.save = drmModeGetCrtc (kms.fd, enc->crtc_id);

    dpy_info.width = kms.mode.hdisplay;
    dpy_info.height = kms.mode.vdisplay;

    drmModeFreeResources (res);

    kms.evctx.page_flip_handler = pageflip_handle;
    kms.evctx.version = DRM_EVENT_CONTEXT_VERSION;
}

static void init_gbm (void) {
    gbm.device = gbm_create_device (kms.fd);
    if (gbm.device == NULL) {
        fprintf (stderr, "failed to create gbm device!\n");
        exit (-1);
    }

    gbm.surface =  gbm_surface_create (
            gbm.device,
            dpy_info.width,
            dpy_info.height,
            GBM_BO_FORMAT_XRGB8888,
            GBM_BO_USE_RENDERING);
    if (gbm.surface == NULL) {
        fprintf (stderr, "failed to create gbm surface!\n");
        exit (-1);
    }

}

static void init_egl (void) {
    EGLint n;
    cairo_status_t st;

    egl.dpy = eglGetDisplay (gbm.device);
    if (egl.dpy == EGL_NO_DISPLAY) {
        fprintf (stderr, "egl get display failed.\n");
        exit (-1);
    }

    if (!eglInitialize (egl.dpy, NULL, NULL)) {
        fprintf (stderr, "egl initialize failed.\n");
        exit (-1);
    }

    eglBindAPI (EGL_OPENGL_API);
    
    if (!eglChooseConfig (egl.dpy, attribs, &egl.config, 1, &n)
            || n != 1) {
        fprintf (stderr, "failed to choose egl config.\n");
        exit (-1);
    }
    
    egl.ctx = eglCreateContext (egl.dpy,
            egl.config, EGL_NO_CONTEXT, NULL);
    if (egl.ctx == NULL) {
        fprintf (stderr, "failed to create context.\n");
        exit (-1);
    }

    egl.surface = eglCreateWindowSurface (egl.dpy,
            egl.config, gbm.surface, NULL);

    egl.c_device = cairo_egl_device_create (egl.dpy, egl.ctx);

    cairo_device_flush (egl.c_device);
    st = cairo_device_acquire (egl.c_device);
    if (st != CAIRO_STATUS_SUCCESS) {
        fprintf (stderr, "failed to acquire device: %s.\n",
                cairo_status_to_string (st));
        exit (-1);
    }
    
    if (!eglMakeCurrent (egl.dpy, egl.surface,
                egl.surface, egl.ctx)) {
        fprintf (stderr, "failed to make context current\n");
        exit (-1);
    }
    
    egl.c_surface = cairo_gl_surface_create_for_egl (
            egl.c_device, egl.surface,
            dpy_info.width, dpy_info.height);
}

static void fb_realease (struct gbm_bo *bo, void *data) {
    struct drm_fb *fb = (struct drm_fb*)data;
    fb->flip = 0;

    if (fb->fb_id)
        drmModeRmFB (kms.fd, fb->fb_id);
    xfree (fb);
}

static struct drm_fb *get_fb (void) {
    struct gbm_bo *bo = gbm_surface_lock_front_buffer (gbm.surface);
    struct drm_fb *fb = gbm_bo_get_user_data (bo);
    int ret;
    uint32_t stride, handle;


    if (fb) 
        return fb;

    fb = xmalloc (sizeof (struct drm_fb));
    fb->bo = bo;
    fb->flip = 1;

    handle = gbm_bo_get_handle (bo).u32;
    stride = gbm_bo_get_stride (bo);

    fprintf (stderr, "handle=%u handle=%u\n", handle, stride);

    ret = drmModeAddFB (kms.fd, 
            dpy_info.width, dpy_info.height,
            24, 32, stride, handle, &fb->fb_id);
    if (ret) {
        fprintf (stderr, "failed to create fb :%s\n",
                strerror (errno));
        return NULL;
    }
    gbm_bo_set_user_data (bo, fb, fb_realease);

    return fb;
}

static void event_handle (int fd, void *data) {
    drmHandleEvent (fd, &kms.evctx);
}

static data_handler_t dht = {
    .data = NULL,
    .handler = event_handle
};

static int renderable (void) {
    return gbm_surface_has_free_buffers (gbm.surface);
}

static cairo_t* plane_create (void) {
    cairo_surface_flush (egl.c_surface);
    return cairo_create (egl.c_surface);
}

static void refresh (void) {
    int ret;
    struct drm_fb *next_fb;

    cairo_gl_surface_swapbuffers (egl.c_surface);
    
    next_fb = get_fb ();

    ret = drmModePageFlip (kms.fd, kms.enc->crtc_id,
            next_fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, kms.fb->bo);
    if (ret) {
        fprintf (stderr, 
                "failed to queue page flip: %s\n",
                strerror (errno));
        return;
    }

    kms.fb = next_fb;

}


screen_t* screen_create (int epoll_fd) {
    screen_t *scr;
    int ret;
    struct drm_fb *fb;
    struct epoll_event epv;
    cairo_t *cr;

    /* initialize kms */
    init_kms ();

    /* initialize gbm */
    init_gbm ();

    /* initialize egl/cairo */
    init_egl ();

    cr = cairo_create (egl.c_surface);
    cairo_paint (cr);
    cairo_destroy (cr);

    cairo_gl_surface_swapbuffers (egl.c_surface);

    kms.fb = get_fb ();

    ret = drmModeSetCrtc (kms.fd,
            kms.enc->crtc_id, kms.fb->fb_id,
            0, 0,
            &kms.con->connector_id, 1,
            &kms.mode);
    if (ret) {
        fprintf (stderr, "failed to set mode: %m\n");
        return NULL;   
    }
    
    /*register epoll*/
    epv.events = EPOLLIN;
    epv.data.fd = kms.fd;
    epv.data.ptr = &dht;

    ret = epoll_ctl (epoll_fd, EPOLL_CTL_ADD, kms.fd, &epv);

    if (ret)
        fprintf (stderr, "failed to add fd", strerror (errno));

    scr = xmalloc (sizeof (screen_t));

    scr->width = dpy_info.width;
    scr->height = dpy_info.height;
    scr->renderable = renderable;
    scr->plane_create = plane_create;
    scr->refresh_screen = refresh;

    return scr;
}

void screen_destory (screen_t *scr, int epoll_fd) {
    int ret;
    struct epoll_event epv;

    /*deregister epoll*/
    epv.events = EPOLLIN;
    epv.data.fd = kms.fd;
    epv.data.ptr = &dht;
    
    ret = epoll_ctl (epoll_fd, EPOLL_CTL_DEL, kms.fd, &epv);

    if (ret) 
        fprintf (stderr, "failed to add fd", strerror (errno));

    /*drm crtc restore */
    drmModeSetCrtc (kms.fd,
            kms.save->crtc_id,
            kms.save->buffer_id,
            kms.save->x,
            kms.save->y,
            &kms.con->connector_id,
            1,
            &kms.save->mode);
    drmModeFreeCrtc (kms.save);
    drmModeFreeConnector (kms.con);
    drmModeFreeEncoder  (kms.enc);


    /*remove frame buffer*/

    gbm_surface_release_buffer (gbm.surface, kms.fb->bo);

    eglMakeCurrent (egl.dpy,
            EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);


    /*egl clear*/
    eglDestroySurface (egl.dpy, egl.surface);
    eglDestroyContext (egl.dpy, egl.ctx);
    eglTerminate (egl.dpy);

    /*cairo clear*/
    cairo_surface_finish (egl.c_surface);
    cairo_device_finish (egl.c_device);

    /*gbm clear*/
    gbm_surface_destroy (gbm.surface);
    gbm_device_destroy (gbm.device);

    /*close fd*/
    close (kms.fd);

    xfree (scr);
}
