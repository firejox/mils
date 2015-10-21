#include "view_private.h"
#include "image.h"
#include "common.h"
#include <glob.h>
#include <stdlib.h>
#include <string.h>

inline void pango_rect_to_view_rect (const PangoRectangle * restrict p_rect,
        view_rectangle_t * restrict v_rect) {
    v_rect->x = pango_units_to_double(p_rect->x);
    v_rect->y = pango_units_to_double(p_rect->y);
    v_rect->width = pango_units_to_double(p_rect->width);
    v_rect->height = pango_units_to_double(p_rect->height);
}


static void setup_background (cairo_t *cr, view_t *v,
        options_t *opts, const char *fn) {

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

        w_scale = (double)v->width / (double)back_im_w;
        h_scale = (double)v->height/ (double)back_im_h;

        cairo_scale (cr, w_scale, h_scale);
        cairo_set_source_surface (cr, background_im, 0, 0);
        cairo_paint (cr);

        cairo_surface_destroy (background_im);
    } else 
        fprintf (stderr, "no background image!\n");
    cairo_restore (cr);
}

static void setup_panel (cairo_t *cr, view_t *v,
             options_t *opts, const char *fn, panel_t *p) {
    const option_data_t *data;
    option_type_t type;
    cairo_surface_t *panel_im;
    
    data = get_option_data (opts, "input_panel_x", &type);
    if (type == OPTION_DOUBLE)
        p->x = v->width * data->fp;
    else
        p->x = data->num;

    data = get_option_data (opts, "input_panel_y", &type);
    if (type == OPTION_DOUBLE)
        p->y = v->height * data->fp;
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

    data = get_option_data (opts, "input_maxlength_name", &type);
    view->input_name.text.str.lt = line_text_create (data->num);
    view->input_name.text.type = LINE_TEXT_TYPE;


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

    data = get_option_data (opts, "input_maxlength_pass", &type);
    view->input_pass.text.str.lt = line_text_create (data->num);

/***************************************************************/
    
    data = get_option_data (opts, "input_color", &type);
    memcpy (&view->input_name.color, &data->color, sizeof (rgb_t));
    memcpy (&view->input_pass.color, &data->color, sizeof (rgb_t));

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
    memcpy (&view->user.color, &data->color, sizeof (rgb_t));

    data = get_option_data (opts, "username_font", &type);
    view->user.font_desc = pango_font_description_from_string 
        (data->str);

    data = get_option_data (opts, "username_msg", &type);
    view->user.text.str.cs = xstrdup (data->str);
    view->user.text.type = CSTRING_TYPE;

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
    memcpy (&view->passwd.color, &data->color, sizeof (rgb_t));

    data = get_option_data (opts, "password_font", &type);
    view->passwd.font_desc = pango_font_description_from_string 
        (data->str);

    data = get_option_data (opts, "password_msg", &type);
    view->passwd.text.str.cs = xstrdup (data->str);
    view->passwd.text.type = CSTRING_TYPE;

}

view_t* load_theme (const char *name, uint32_t width, uint32_t height) {
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

    view->width = width;
    view->height = height;


    opts = options_create (desc_f);
    xfree (desc_f);

    theme = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
            width, height);
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
        setup_background (cr, view, opts, g_theme.gl_pathv[0]);
    } 
    globfree (&g_theme);
    xfree (background_g);

    /*load panel image*/
    glob (panel_g, GLOB_BRACE, NULL, &g_theme);
    if (g_theme.gl_pathc) {
        fprintf (stderr, "file: %s\n", g_theme.gl_pathv[0]);
        setup_panel (cr, view, opts, g_theme.gl_pathv[0], &panel);
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

static char* string_t_to_c_str (view_string_t *str) {
    switch (str->type) {
        case CSTRING_TYPE:
            return xstrdup(str->str.cs);
        case LINE_TEXT_TYPE:
            return line_text_to_string (str->str.lt);
    }
}

static void render_cursor (cairo_t *cr,
        PangoLayout *layout, view_text_t *text) {
    PangoRectangle scur;
    view_rectangle_t vcur;
    int idx;
    if (text->text.type == LINE_TEXT_TYPE) {
        idx = line_text_cursor_pos (text->text.str.lt);
        cairo_save (cr);
        pango_layout_get_cursor_pos (layout, idx, &scur, NULL);

        pango_rect_to_view_rect (&scur, &vcur);


        cairo_rectangle (cr, vcur.x, vcur.y, vcur.width, vcur.height);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
        

        cairo_restore (cr);
    }
}

void render_text (cairo_t *cr, view_text_t *text) {
    PangoLayout *layout;
    char *str = string_t_to_c_str (&text->text);

    cairo_save (cr);

    cairo_translate (cr, text->x, text->y);
    
    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, str, -1);
    pango_layout_set_font_description (layout, text->font_desc);

    cairo_set_source_rgb (cr, text->color.r, text->color.g, text->color.b);
    render_cursor (cr, layout, text);
    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);


    g_object_unref (layout);

    cairo_restore (cr);
    xfree (str);
}
