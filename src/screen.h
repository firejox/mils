#ifndef _SCREEN_H
#define _SCREEN_H

/**cairo header*/
#include <cairo.h>
#include <cairo-gl.h>

typedef struct _screen {
    uint32_t width, height;
    int (*renderable) (void);
    cairo_t* (*plane_create) (void);
    void (*refresh_screen) (void);
} screen_t;

screen_t* screen_create (int epoll_fd);
void screen_destory (screen_t* scr, int epoll_fd);


#endif
