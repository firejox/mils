//#include "view.h"
#include "input.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int epfd;
    struct epoll_event tmp;
    data_handler_t *handle;
    int timeout = -1;
    int ret;
    keysym_t syms;
//    view_t *v;
    input_state_t *ist;
    char *str;

    epfd = epoll_create1 (0);
    if (epfd == -1) {
        perror ("epoll create");
        return -1;
    }

    ist = input_state_create (epfd);
    while ((ret = epoll_wait (epfd, &tmp, 1, timeout)) > -1) {
        if (ret) {
            handle = tmp.data.ptr;
            handle->handler (tmp.data.fd, handle->data);
        }

        syms = get_input_state_info (ist, &timeout);
        if (syms != XKB_KEY_NoSymbol)
            fprintf (stderr, "key : %x\n", syms);

    }


    
/*
    screen_enter (epfd);

    
    v = view_create ();
    view_set_input (v, "hello", VIEW_USER);
    view_update (v, VIEW_USER);
    
    while (epoll_wait (epfd, &tmp, 1, -1))
        break;
    handle = tmp.data.ptr;
    handle->handler (tmp.data.fd, handle->data);
    
    getchar();
    fprintf (stderr, "hello\n");
    
    view_destory (v);
    
    screen_leave(epfd);
*/
    close (epfd);
  
    return 0;
}

