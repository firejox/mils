#include "view.h"
#include "input.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>




int main(void) {
    int epfd, ret;
    struct epoll_event tmp;
    input_state_t *ist;
    data_handler_t *dht;
    view_t *v;
    line_text_t *lt;
    int timeout = -1;
    keysym_t sym;

    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror ("epoll create failed");
        exit(-1);
    }

    screen_enter (epfd);
    ist = input_state_create (epfd);
    v = view_create ();
    lt = view_user_input_ref (v);

    view_update (v, VIEW_USER);
    while ((ret = epoll_wait (epfd, &tmp, 1, timeout)) > -1) {
        if (ret) {
            dht = tmp.data.ptr;
            dht->handler (dht->fd, dht->data);
        }

        
       sym = get_input_state_info (ist, &timeout);
       switch (sym) {
           case XKB_KEY_NoSymbol:
               break;
           case XKB_KEY_Return:
               goto input_end;
           case XKB_KEY_BackSpace:
               line_text_handle_event (lt, DELETE_LEFT_CH);
               break;
           case XKB_KEY_Delete:
               line_text_handle_event (lt, DELETE_RIGHT_CH);
               break;
           case XKB_KEY_Left:
               line_text_handle_event (lt, CURSOR_MOVE_LEFT);
               break;
           case XKB_KEY_Right:
               line_text_handle_event (lt, CURSOR_MOVE_RIGHT);
               break;
           default:
               line_text_handle_event (lt, APPEND_CH, keysym_to_utf32(sym));
               break;
       }
       if (sym != XKB_KEY_NoSymbol)
           view_update (v, VIEW_USER);
    
    }

input_end:

    line_text_unref (lt);

    view_destory (v);
    input_state_destroy (ist);
    screen_leave (epfd);
    return 0;
}

