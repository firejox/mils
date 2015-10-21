
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <libinput.h>
#include <math.h>
#include <time.h>


#include "common.h"
#include "input.h"
#include "keytree.h"

#define LIBINPUT_OFFSET 8

struct _input_state {
    const char *seat;
    char *str;
    
    struct xkb_context *ctx;
    struct xkb_rule_names names;
    struct xkb_keymap *keymap;
    struct xkb_state *st;
    keysym_t sym;

    key_tree_t *ktree;
    

    struct udev *dev;
    struct libinput *li;

    int delay; /*milliseconds*/
    int rate; /**millisecond*/

    uint64_t last_time;
    int repeat_state;

    int epfd;
    int fd;
    data_handler_t handler;

};

static int open_input 
        (const char *path, int flags, void *data) {
    int fd;
    fprintf (stderr, "open file : %s\n", path);
    fd = open (path, flags);
    if (fd < 0)
        fprintf (stderr, "open %s failed.", path);

    return fd;
}

static void close_input (int fd, void *data) {
    close (fd);
}

static struct  libinput_interface face = {
    .open_restricted = open_input,
    .close_restricted = close_input
};


#define IS_KEYBOARD_DEVICE(dev) \
    (libinput_device_has_capability \
     (dev, LIBINPUT_DEVICE_CAP_KEYBOARD) &&\
     libinput_device_keyboard_has_key (dev, KEY_ESC))



static void device_add (struct libinput_event *ev) {
    struct libinput_device *dev;
    dev = libinput_event_get_device (ev);
    
    fprintf (stderr, "device : %s\n",
            libinput_device_get_sysname (dev));
    if (!IS_KEYBOARD_DEVICE (dev)) 
        libinput_device_config_send_events_set_mode
            (dev, LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
}

/*handle input event from fd*/
static void input_handler (int fd, void *data) {
    input_state_t *ist = data;
    struct libinput_event *ev;
    struct libinput_event_keyboard *kev;
    enum xkb_key_direction dv;
    xkb_keycode_t keycode, key;
    int count = 0;


    libinput_dispatch (ist->li);

    while ((ev = libinput_get_event (ist->li))) {
        switch (libinput_event_get_type (ev)) {
            case LIBINPUT_EVENT_DEVICE_ADDED:
                device_add (ev);

                ist->last_time = xget_current_time();
                break;
            case LIBINPUT_EVENT_KEYBOARD_KEY:
                kev = libinput_event_get_keyboard_event (ev);

                keycode = libinput_event_keyboard_get_key (kev) +
                    LIBINPUT_OFFSET;

                ist->last_time = xget_current_time();

                dv = libinput_event_keyboard_get_key_state (kev);


                keytree_update_key (ist->ktree, keycode, dv);

                xkb_state_update_key (ist->st, keycode, dv);
                if (dv && xkb_keymap_key_repeats (ist->keymap, keycode)) 
                    ist->repeat_state = 0;
                


                keycode = keytree_get_top_key (ist->ktree);
                ist->sym = xkb_state_key_get_one_sym (ist->st, keycode);
                

                break;
            default:
                break;
        }
        libinput_event_destroy (ev);
        count++;
    }

}

static void init_libinput (input_state_t *ist) {
    
    ist->seat = "seat0";
    ist->dev  = udev_new ();
    ist->li   = libinput_udev_create_context (&face, NULL, ist->dev);
    ist->last_time = 0;

    ist->delay = 200;
    ist->rate  = 5;
    ist->repeat_state = 0;

    libinput_udev_assign_seat (ist->li, ist->seat);

    ist->fd = libinput_get_fd (ist->li);
    
    ist->handler.fd = ist->fd;
    ist->handler.data = ist;
    ist->handler.handler = input_handler;

}

static void init_xkb (input_state_t *ist) {
    ist->ctx = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    if (!ist->ctx) {
        fprintf (stderr, "xkb context new failed.\n");
        return;
    }

    /*keyboard information*/
    ist->names.rules = NULL;
    ist->names.model = NULL;
    ist->names.layout = NULL;
    ist->names.variant = NULL;
    ist->names.options = NULL;

    ist->keymap = xkb_keymap_new_from_names (ist->ctx, 
            &ist->names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!ist->keymap) {
        fprintf (stderr, "xkb keymap new failed.\n");
        return;
    }


    ist->ktree = keytree_create (ist->keymap);

    ist->st = xkb_state_new (ist->keymap);
    if (!ist->st) {
        fprintf (stderr, "xkb state new failed.\n");
        return;
    }

    ist->sym = XKB_KEY_NoSymbol;

}

input_state_t* input_state_create (int epollfd) {
    input_state_t *ist = xmalloc (sizeof (input_state_t));
    struct epoll_event epv;

    ist->epfd = epollfd;

    init_xkb (ist);
    init_libinput (ist);

    epv.events = EPOLLIN|EPOLLOUT;
    epv.data.fd = ist->fd;
    epv.data.ptr = &ist->handler;
    

    if (epoll_ctl (epollfd, EPOLL_CTL_ADD, ist->fd, &epv) == -1) {
        perror ("epoll_ctl_add failed");
        xfree (ist);
        return NULL;
    }

    fprintf (stderr, "input fd : %d\n",ist->fd);

    return ist;
}


keysym_t get_input_state_info
            (input_state_t *ist, int *timeout) {
    int size;
    int clock;
    xkb_keycode_t key;

    clock = xget_current_time ();
    
    /* 
     *  start press   =>repeate rate
     *  |            |
     *  ->---------->-->-->-->key press time
     *      |
     *      ->delay time
     * */

    if (ist->repeat_state == 2) {
        if ((clock - ist->last_time) < ist->rate) {
            *timeout = ist->rate + ist->last_time - clock;
            return XKB_KEY_NoSymbol;
        }
        else {
            *timeout = ist->rate;
            ist->last_time = clock;
        }
    } else if (ist->repeat_state == 1) {
        if ((clock - ist->last_time) < ist->delay) {
            *timeout = ist->delay + ist->last_time - clock;
            return XKB_KEY_NoSymbol;
        }
        else {
            *timeout = ist->rate;
            ist->last_time = clock;
            ist->repeat_state = 2;
        }
    } else {
        ist->repeat_state = 1;
    }

    return ist->sym;
}

int keysym_to_utf8 (keysym_t sym, char *buf, size_t sz) {
    return xkb_keysym_to_utf8 (sym, buf, sz);
}

uint32_t keysym_to_utf32 (keysym_t sym) {
    return xkb_keysym_to_utf32 (sym);
}

void input_state_destroy (input_state_t *ist) {
    struct epoll_event epv;

    keytree_destroy (ist->ktree);

    xkb_state_unref (ist->st);
    xkb_keymap_unref (ist->keymap);
    xkb_context_unref (ist->ctx);

    epoll_ctl (ist->epfd, EPOLL_CTL_DEL, ist->fd, &epv);

    libinput_unref (ist->li);
    udev_unref (ist->dev);

    xfree (ist);

}
