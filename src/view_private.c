#include "view.h"
#include "view_private.h"
#include "image.h"
#include "options.h"
#include "common.h"

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

view_t* load_theme (const char *name) {
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

char* string_t_to_c_str (string_t *str) {
    switch (str->type) {
        case CSTRING_TYPE:
            return xstrdup(str->str.cs);
        case LINE_TEXT_TYPE:
            return line_text_to_string (str->str.lt);
    }
}
