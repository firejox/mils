#include <security/pam_appl.h>
#include <security/pam_misc.h>


#include "auth.h"
#include "view.h"
#include "input.h"
#include "common.h"
#include "utils.h"


struct _auth {
    int epfd;
    
    view_t *v;
    input_state_t *ist;
    
    line_text_t *user;
    line_text_t *passwd;

    pam_handle_t *pamh;
    struct pam_conv pconv;

    const char *name;
};



static int auth_conv (int num_msg,
        const struct pam_message **msgm,
        struct pam_response **resp,
        void *data) {
    auth_t *au = (auth*)data;
    

}


auth_t* auth_create (const char *serve_name) {
    auth_t *au = xmalloc (sizeof (auth_t));

    au->pconv.conv = auth_conv;
    au->pconv.appdata_ptr = au;

    au->pamh = NULL;
    
    
    au->ist = NULL;
    
    au->v = view_create();
    /*user line text ref*/
    au->user = view_user_input_ref (au->v);

    /**passwd line text ref*/
    au->passwd = view_pass_input_ref (au->v);


    au->epfd = epoll_create1 (0);
    if (au->epfd == -1) {
        fprintf (stderr, "epoll create failed: %s\n", 
                strerror (errno));
        exit (-1);
    }
    

    au->name = xstrdup (serve_name);

    return au;
}

void auth_once (auth *au) {
    int res;
    
    res = pam_start (au->name, NULL, &au->pconv, &au->pamh);


    if (res != PAM_SUCCESS) 
        goto pam_start_failed;

    screen_enter (au->epfd);
    au->ist = input_state_create (au->epfd);


    res = pam_authenticate (au->pamh, 0);
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;

    res = pam_acct_mgmt (au->pamh, 0)
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;
    
    fprintf (stderr, "Authenticated.\n");


    goto pam_auth_clean;

pam_auth_failed:
    fprintf (stderr, "Not Authenticated.\n");

pam_auth_clean:
    input_state_destroy (au->ist);
    screen_leave (au->epfd);

pam_start_failed:
    pam_end (au->pamh, res);
}


