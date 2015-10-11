
#ifndef _VIEW_H
#define _VIEW_H
#include "utils.h"

typedef struct _view view_t;

typedef enum {
    VIEW_USER,
    VIEW_PASSWD,
    VIEW_REBOOT,
    VIEW_POWEROFF,
    VIEW_SUSPEND,
} view_event_type;


/*set up graphic enviroment*/
void screen_enter   (int epoll_fd);
void screen_leave   (int epoll_fd);


/*load theme and create login srenn*/
view_t* view_create  (void);

line_text_t* view_user_input_ref(view_t *view);
line_text_t* view_pass_input_ref(view_t *view);

void    view_update (view_t *view, view_event_type type);

void    view_destory (view_t *view);



#endif
