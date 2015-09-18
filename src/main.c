#include "view.h"
#include "input.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <unistr.h>
#include <assert.h>
#include <errno.h>



static void test__show_text (view_t *v, uint32_t *ustr, size_t len) {
    size_t len2;
    uint8_t *u8str;
    int i;

    if (len + 1) {
        
        u32_to_u8 (ustr, len + 2, NULL, &len2);

        u8str = xmalloc (len2);
        u32_to_u8 (ustr, len + 2, u8str, &len2);

        view_set_input (v, u8str, VIEW_USER);

        xfree (u8str); 
    } else 
        view_set_input (v, "", VIEW_USER);
    
    view_update (v, VIEW_USER);
}

int main(void) {
    int epfd;
    struct epoll_event tmp;
    data_handler_t *handle;
    int timeout = -1;
    int ret;
    keysym_t sym;
    view_t *v;
    input_state_t *ist;

    uint8_t *u8str;

    uint32_t *ustr;
    int lim, cur = -1;

    epfd = epoll_create1 (0);
    if (epfd == -1) {
        perror ("epoll create");
        return -1;
    }

    
    ist = input_state_create (epfd);
    screen_enter (epfd);

    
    v = view_create ();
    lim = view_get_input_lim (v, VIEW_USER);
    ustr = xmalloc (sizeof (uint32_t) * (lim + 4));
    ustr[0] = ustr[1] = 0;

    view_update (v, VIEW_USER); 
    while ((ret = epoll_wait (epfd, &tmp, 1, timeout)) > -1) {
        if (ret) {
            handle = tmp.data.ptr;
            handle->handler (handle->fd, handle->data);
        }
        sym = get_input_state_info (ist, &timeout);

        switch (sym) {
            case XKB_KEY_NoSymbol:
                break;
            case XKB_KEY_Return:
                goto end_view;
            
            case XKB_KEY_BackSpace:
                if (cur >= 0) {
                    ustr[cur --] = 0;
                }
                test__show_text (v, ustr, cur); 
                break;

            default :
                if (cur < lim) {
                    ustr[++cur] = keysym_to_utf32 (sym);
                    ustr[cur + 1] = 0;
                }
                test__show_text (v, ustr, cur); 
                break;
        }



    }

end_view:
    
    
    view_destory (v);
    
    screen_leave(epfd);

    close (epfd);
  
    return 0;
}

