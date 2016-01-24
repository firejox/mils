#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <errno.h>

#include "auth.h"
#include "view.h"
#include "input.h"
#include "common.h"
#include "utils.h"



struct _auth {
    int epfd;
    
    view_t *v;
    input_state_t *ist;
    
    view_event_type cur_v; /*current view status*/
    line_text_t *cur_lt; /*current line input*/

    pam_handle_t *pamh;
    struct pam_conv pconv;

    const char *name;
};


static char* retrieve_input (auth_t *au) {
    int timeout = -1, ret;
    data_handler_t *dht;
    struct epoll_event tmp;
    keysym_t sym;
    char *str;

    view_update (au->v, au->cur_v);
    while ((ret = epoll_wait (au->epfd, &tmp, 1, timeout)) > -1) {
        if (ret) {
            dht = tmp.data.ptr;
            dht->handler (dht->fd, dht->data);
        }
       
       sym = get_input_state_info (au->ist, &timeout);
       switch (sym) {
           case XKB_KEY_NoSymbol:
               break;
           case XKB_KEY_Return:
               goto input_end;
           case XKB_KEY_BackSpace:
               line_text_handle_event (au->cur_lt, DELETE_LEFT_CH);
               break;
           case XKB_KEY_Delete:
               line_text_handle_event (au->cur_lt, DELETE_RIGHT_CH);
               break;
           case XKB_KEY_Left:
               line_text_handle_event (au->cur_lt, CURSOR_MOVE_LEFT);
               break;
           case XKB_KEY_Right:
               line_text_handle_event (au->cur_lt, CURSOR_MOVE_RIGHT);
               break;
           default:
               line_text_handle_event (au->cur_lt, APPEND_CH, keysym_to_utf32(sym));
               break;
       }
       if (sym != XKB_KEY_NoSymbol)
           view_update (au->v, au->cur_v);
    
    }

input_end:
    str = line_text_to_string (au->cur_lt);
    line_text_unref (au->cur_lt);
    return str;
}

static int auth_conv (int num_msg,
        const struct pam_message **msgm,
        struct pam_response **resp,
        void *data) {
    int i;
    auth_t *au = (auth_t*)data;
    struct pam_response *reply;
    char *str;
    
    reply = xcalloc (num_msg, sizeof (struct pam_response));
    
    for (i = 0; i < num_msg; i++) {
        str = NULL;

        switch (msgm[i]->msg_style) {
            case PAM_PROMPT_ECHO_ON: /*user*/
                au->cur_v = VIEW_USER;
                au->cur_lt = view_user_input_ref (au->v);
                str = retrieve_input (au);
                break;
            case PAM_PROMPT_ECHO_OFF:
                au->cur_v = VIEW_USER;
                au->cur_lt = view_pass_input_ref (au->v);
                str = retrieve_input (au);
                break;

            case PAM_ERROR_MSG:
                if (fprintf (stderr,
                            "error msg %s\n", msgm[i]->msg) < 0) 
                    goto failed_conv;

                break;
                
            case PAM_TEXT_INFO:
                if (fprintf (stderr,
                            "text info %s\n", msgm[i]->msg) < 0)
                    goto failed_conv;
                break;

            default :
                goto failed_conv;
        }

        if (str) {
            reply[i].resp_retcode = 0;
            reply[i].resp = str;
        
        }
        
    }

    *resp = reply;

    return PAM_SUCCESS;

failed_conv:
    
    if (reply) {
        for (i = 0; i < num_msg; i++) {
            if (reply[i].resp == NULL)
                continue;

            switch (msgm[i]->msg_style) {
                case PAM_PROMPT_ECHO_ON:
                case PAM_PROMPT_ECHO_OFF:
                    free(reply[i].resp);
            }

            reply[i].resp = NULL;
        }
    }
    
    return PAM_CONV_ERR;

}


auth_t* auth_create (const char *serve_name) {
    auth_t *au = xmalloc (sizeof (auth_t));

    au->pconv.conv = auth_conv;
    au->pconv.appdata_ptr = au;

    au->pamh = NULL;
    
    
    au->ist = NULL;

    au->epfd = epoll_create1 (0);
    if (au->epfd == -1) {
        fprintf (stderr, "epoll create failed: %s\n", 
                strerror (errno));
        exit (-1);
    }
    
    screen_enter (au->epfd);
    
    au->v = view_create();

    au->name = xstrdup (serve_name);

    return au;
}

void auth_once (auth_t *au) {
    int res;
    
    res = pam_start (au->name, NULL, &au->pconv, &au->pamh);

    if (res != PAM_SUCCESS) 
        goto pam_start_failed;

    au->ist = input_state_create (au->epfd);


    res = pam_authenticate (au->pamh, 0);
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;

    res = pam_acct_mgmt (au->pamh, 0);
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;
    
    fprintf (stderr, "Authenticated.\n");


    goto pam_auth_clean;

pam_auth_failed:
    fprintf (stderr, "Not Authenticated.\n");

pam_auth_clean:
    input_state_destroy (au->ist);

pam_start_failed:
    pam_end (au->pamh, res);
}


void auth_loop (auth_t *au) {
    while (1) {
        auth_once (au);
    }
}

void auth_destory (auth_t *au) {
    screen_leave (au->epfd);
    view_destory (au->v);
    close(au->epfd);
    xfree (au);
}
