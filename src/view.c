#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glob.h>


/*pango header*/
#include <pango/pangocairo.h>

#include "view.h"
#include "common.h"
#include "image.h"
#include "options.h"
#include "screen.h"


typedef struct _text {
    double x, y;
    double r, g, b;
    const char *text;
    PangoFontDescription *font_desc;
} text_t;

typedef struct _panel {
    double x, y;
    double w, h;
} panel_t;

struct _view {
    cairo_surface_t *theme;
    text_t input_name;
    text_t input_pass;
    text_t user;
    text_t passwd;
};

static screen_t *scr;

void screen_enter (int epoll_fd) {
    scr = screen_create (epoll_fd);    
}

void screen_leave (int epoll_fd) {
    screen_destory (scr, epoll_fd);
}

static void setup_background 
        (cairo_t *cr, options_t *opts, const char *fn) {
    cairo_surface_t *background_im;
    int back_im_w, back_im_h;
    double w_scale, h_scale;
    const option_data_t *data;
    option_type_t type;
    
    cairo_save (cr);
    data = get_option_data (opts, "background_color", &type);
    cairo_set_source_rgb (cr,
            data->color.r, data->color.g, data->color.b);
    cairo_fill (cr);

    background_im = get_image_surface (fn);    
    if (background_im) {
        back_im_w = cairo_image_surface_get_width (background_im);
        back_im_h = cairo_image_surface_get_height (background_im);

        w_scale = (double)scr->width / (double)back_im_w;
        h_scale = (double)scr->height/ (double)back_im_h;

        cairo_scale (cr, w_scale, h_scale);
        cairo_set_source_surface (cr, background_im, 0, 0);
        cairo_paint (cr);

        cairo_surface_destroy (background_im);
    } else 
        fprintf (stderr, "no background image!\n");
    cairo_restore (cr);
}

static void setup_panel 
            (cairo_t *cr, options_t *opts, const char *fn,
             panel_t *p) {
    const option_data_t *data;
    option_type_t type;
    cairo_surface_t *panel_im;
    
    data = get_option_data (opts, "input_panel_x", &type);
    if (type == OPTION_DOUBLE)
        p->x = scr->width * data->fp;
    else
        p->x = data->num;

    data = get_option_data (opts, "input_panel_y", &type);
    if (type == OPTION_DOUBLE)
        p->y = scr->height * data->fp;
    else
        p->y = data->num;

    panel_im = get_image_surface (fn);
    if (panel_im) {
        p->w = (double)cairo_image_surface_get_width (panel_im);
        p->h = (double)cairo_image_surface_get_height (panel_im);
        p->x -= p->w/2.0;
        p->y -= p->h/2.0;

        cairo_save (cr);
        cairo_set_source_surface (cr, panel_im, p->x, p->y);
        cairo_paint (cr);
        cairo_restore (cr);

        cairo_surface_destroy (panel_im);
    } else 
        fprintf (stderr, "no panel image!\n");
}


static void setup_input
            (view_t *view, options_t *opts, const panel_t *p) {
    const option_data_t *data;
    option_type_t type;


    data = get_option_data (opts, "input_name_x", &type);
    if (type == OPTION_DOUBLE)
        view->input_name.x = p->x + p->w * data->fp;
    else
        view->input_name.x = p->x + data->num;

    data = get_option_data (opts, "input_name_y", &type);
    if (type == OPTION_DOUBLE)
        view->input_name.y = p->y + p->h * data->fp;
    else
        view->input_name.y = p->y + data->num;

/**************************************************************/

    data = get_option_data (opts, "input_pass_x", &type);
    if (type == OPTION_DOUBLE)
        view->input_pass.x = p->x + p->w * data->fp;
    else if (data->num != -1)
        view->input_pass.x = p->x + data->num;
    else
        view->input_pass.x = view->input_name.x;

    data = get_option_data (opts, "input_pass_y", &type);
    if (type == OPTION_DOUBLE)
        view->input_pass.y = p->y + p->h * data->fp;
    else if (data->num != -1)
        view->input_name.y = p->y + data->num;
    else
        view->input_pass.y = view->input_name.y;

/***************************************************************/
    
    data = get_option_data (opts, "input_color", &type);
    view->input_name.r = view->input_pass.r = data->color.r;
    view->input_name.g = view->input_pass.g = data->color.g;
    view->input_name.b = view->input_pass.b = data->color.b;

    data = get_option_data (opts, "input_font", &type);
    view->input_name.font_desc = pango_font_description_from_string 
        (data->str);

    view->input_pass.font_desc = pango_font_description_from_string 
        (data->str);


}

static void setup_userhint 
        (view_t *view, options_t *opts, panel_t *p) {
    const option_data_t *data;
    option_type_t type;


    data = get_option_data (opts, "username_x", &type);
    if (type == OPTION_DOUBLE)
        view->user.x = p->x + p->w * data->fp;
    else
        view->user.x = p->x + data->num;

    data = get_option_data (opts, "username_y", &type);
    if (type == OPTION_DOUBLE)
        view->user.y = p->y + p->h * data->fp;
    else
        view->user.y = p->y + data->num;

    data = get_option_data (opts, "username_color", &type);
    view->user.r = data->color.r;
    view->user.g = data->color.g;
    view->user.b = data->color.b;

    data = get_option_data (opts, "username_font", &type);
    view->user.font_desc = pango_font_description_from_string 
        (data->str);

    data = get_option_data (opts, "username_msg", &type);
    view->user.text = xstrdup (data->str);
}

static void setup_passhint 
        (view_t *view, options_t *opts, panel_t *p) {
    const option_data_t *data;
    option_type_t type;


    data = get_option_data (opts, "password_x", &type);
    if (type == OPTION_DOUBLE)
        view->passwd.x = p->x + p->w * data->fp;
    else
        view->passwd.x = p->x + data->num;

    data = get_option_data (opts, "password_y", &type);
    if (type == OPTION_DOUBLE)
        view->passwd.y = p->y + p->h * data->fp;
    else
        view->passwd.y = p->y + data->num;

    data = get_option_data (opts, "password_color", &type);
    view->passwd.r = data->color.r;
    view->passwd.g = data->color.g;
    view->passwd.b = data->color.b;

    data = get_option_data (opts, "password_font", &type);
    view->passwd.font_desc = pango_font_description_from_string 
        (data->str);

    data = get_option_data (opts, "password_msg", &type);
    view->passwd.text = xstrdup (data->str);
}

static view_t* load_theme (const char *name) {
    view_t *view = xmalloc (sizeof (view_t));

    cairo_surface_t *theme;
    cairo_status_t st;
    cairo_t *cr;

    glob_t g_theme;
    char *desc_f = xsprintf ("%s/mils.theme", name);
    char *background_g = xsprintf ("%s/background.{"
            AVAILABLE_IMAGE_FORMAT"}", name);
    char *panel_g = xsprintf ("%s/panel.{"
            AVAILABLE_IMAGE_FORMAT"}", name);

    options_t *opts;
    const option_data_t *data;
    option_type_t type;

    panel_t panel;


    opts = options_create (desc_f);
    xfree (desc_f);

    theme = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
            scr->width, scr->height);
    st = cairo_surface_status (theme);
    if (st != CAIRO_STATUS_SUCCESS) {
        fprintf (stderr, "create theme error: %s\n",
                cairo_status_to_string (st));
        exit (-1);
    }

    cairo_surface_flush (theme);
    cr = cairo_create (theme);
    
    /*load background image*/
    glob (background_g, GLOB_BRACE, NULL, &g_theme);
    if (g_theme.gl_pathc) {
        fprintf (stderr, "file: %s\n", g_theme.gl_pathv[0]);
        setup_background (cr, opts, g_theme.gl_pathv[0]);
    } 
    globfree (&g_theme);
    xfree (background_g);

    /*load panel image*/
    glob (panel_g, GLOB_BRACE, NULL, &g_theme);
    if (g_theme.gl_pathc) {
        fprintf (stderr, "file: %s\n", g_theme.gl_pathv[0]);
        setup_panel (cr, opts, g_theme.gl_pathv[0], &panel);
    }
    globfree (&g_theme);
    xfree (panel_g);

    /*setup input , user hint, passwd hint*/
    setup_input (view, opts, &panel);
    setup_userhint (view, opts, &panel);
    setup_passhint (view, opts, &panel);

    options_destroy (opts);
    cairo_destroy (cr);

    view->theme = theme;
    
    return view;
}



static void render_text (cairo_t *cr, text_t *text) {
    PangoLayout *layout;

    cairo_save (cr);

    cairo_translate (cr, text->x, text->y);
    
    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, text->text, -1);
    pango_layout_set_font_description (layout, text->font_desc);

    cairo_set_source_rgb (cr, text->r, text->g, text->b);
    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    g_object_unref (layout);

    cairo_restore (cr);
}


view_t* view_create (void) {
    view_t* view = load_theme ("default");

    return view;
}

void view_set_input 
    (view_t *view, const char *text,
     view_event_type type) {
    
    switch (type) {
        case VIEW_USER:
            view->input_name.text = text;
            break;
        case VIEW_PASSWD:
            view->input_pass.text = text;
            break;
        default:
            break;
    }
    
}

static int is_single (view_t *v) {
    return (equal(v->input_name.x, v->input_pass.x) &&
            equal(v->input_name.y, v->input_pass.y));
}

void view_update (view_t *view, view_event_type type) {
    cairo_t *wincr;

    if (scr->renderable()) {
    
        wincr = scr->plane_create ();

        cairo_set_source_surface (wincr, view->theme, 
                0, 0);
        cairo_paint (wincr);
        
        switch (type) {
            case VIEW_USER:
                render_text (wincr, &view->user);
                render_text (wincr, &view->input_name);
                break;

            case VIEW_PASSWD:
                if (!is_single (view))
                    render_text (wincr, &view->user);
                    render_text (wincr, &view->input_name);
                render_text (wincr, &view->passwd);
                render_text (wincr, &view->input_pass);
                break;
            default:
                break;
        }

        cairo_destroy (wincr);

        scr->refresh_screen ();
    }
}

void view_destory (view_t *v) {
    cairo_surface_destroy (v->theme);
    pango_font_description_free (v->input_name.font_desc);
    pango_font_description_free (v->input_pass.font_desc);

    xfree ((char*)v->user.text);
    pango_font_description_free (v->user.font_desc);
    xfree ((char*)v->passwd.text);
    pango_font_description_free (v->passwd.font_desc);

    xfree (v);
}
