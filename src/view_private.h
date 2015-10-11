#ifndef __S_VIEW_PRIVATE_H
#define __S_VIEW_PRIVATE_H

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
        line_text_t* lt,
        char*        cs,
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

struct _view {
    cairo_surface_t *theme;
    view_text_t input_name;
    view_text_t input_pass;
    view_text_t user;
    view_text_t passwd;
};

extern struct _view* load_theme (const char *);
extern void render_text (cairo_t*, view_text_t*);

#endif
