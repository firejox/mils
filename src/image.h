#ifndef _IMAGE_H
#define _IMAGE_H

#include <cairo/cairo.h>

#define AVAILABLE_IMAGE_FORMAT "png,"\
    "jpeg,jpg"
extern cairo_surface_t* get_image_surface (const char *fn);

#endif
