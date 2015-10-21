#ifndef __S_VIEW_PRIVATE_H
#define __S_VIEW_PRIVATE_H

#include "view.h"

/*pango header*/
#include <pango/pangocairo.h>


#include "utils.h"
#include "options.h"


typedef enum {
    LINE_TEXT_TYPE,
    CSTRING_TYPE,
} view_string_type;

typedef struct {
    view_string_type type;
    union {
        line_text_t* lt;
        char*        cs;
    } str;
} view_string_t;

typedef struct _view_text {
    double x, y;
    rgb_t color;
    view_string_t text;
    PangoFontDescription *font_desc;
} view_text_t;

typedef struct _panel {
    double x, y;
    double w, h;
} panel_t;


typedef struct _view_rectangle {
    double x;
    double y;
    double width;
    double height;
} view_rectangle_t;

struct _view {
    cairo_surface_t *theme;
    view_text_t input_name;
    view_text_t input_pass;
    view_text_t user;
    view_text_t passwd;
    uint32_t width;
    uint32_t height;
};

extern inline void pango_rect_to_view_rect (const PangoRectangle * restrict p_rect,
        view_rectangle_t * restrict v_rect);


extern struct _view* load_theme (const char *,
        uint32_t, uint32_t);

extern void render_text (cairo_t*, view_text_t*);

#endif
