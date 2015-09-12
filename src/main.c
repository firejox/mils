#include "view.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int epfd;
    struct epoll_event tmp;
    data_handler_t *handle;
    view_t *v;

    epfd = epoll_create1 (0);
    if (epfd == -1) {
        perror ("epoll create");
        return -1;
    }

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

    close (epfd);
  
    return 0;
}

