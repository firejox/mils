#ifndef __S_VIEW_PRIVATE_H
#define __S_VIEW_PRIVATE_H

/*pango header*/
#include <pango/pangocairo.h>
#include "utils.h"


typedef enum {
    LINE_TEXT_TYPE,
    CSTRING_TYPE,
} string_type;

typedef struct {
    string_type type;
    union {
        line_text_t* lt,
        char*        cs,
    } str;
} string_t;

typedef struct _text {
    double x, y;
    rgb_t color;
    string_t text;
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

extern struct _view* load_theme (const char *);
extern char* string_t_to_c_str (string_t *);

#endif
