#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "screen.h"
#include "common.h"
#include "view_private.h"



static screen_t *scr = NULL;

void screen_enter (int epoll_fd) {
    if (scr == NULL)
        scr = screen_create (epoll_fd);
}

void screen_leave (int epoll_fd) {
    if (scr != NULL) {
        screen_destory (scr, epoll_fd);
        scr = NULL;
    }
}



view_t* view_create (void) {
    assert (scr != NULL);
    view_t* view = load_theme ("default", scr->width, scr->height);

    return view;
}

line_text_t* view_user_input_ref (view_t *view) {
    return line_text_ref (view->input_name.text.str.lt);
}

line_text_t* view_pass_input_ref (view_t *view) {
    return line_text_ref (view->input_pass.text.str.lt);
}

static int is_single (view_t *v) {
    return (equal(v->input_name.x, v->input_pass.x) &&
            equal(v->input_name.y, v->input_pass.y));
}

void view_update (view_t *view, view_event_type type) {
    cairo_t *wincr;

    assert (scr != NULL);

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

    line_text_unref (v->input_name.text.str.lt);
    pango_font_description_free (v->input_name.font_desc);
    
    line_text_unref (v->input_pass.text.str.lt);
    pango_font_description_free (v->input_pass.font_desc);

    xfree (v->user.text.str.cs);
    pango_font_description_free (v->user.font_desc);
    xfree (v->passwd.text.str.cs);
    pango_font_description_free (v->passwd.font_desc);

    xfree (v);
}
