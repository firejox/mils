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

    /**passwd line text ref*/


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


    res = pam_authenticate (au->pamh, 0);
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;

    res = pam_acct_mgmt (au->pamh, 0)
    if (res != PAM_SUCCESS)
        goto pam_auth_failed;
    
    fprintf (stderr, "Authenticated.\n");


    screen_leave (au->epfd);
    
    pam_end (au->pamh, res);
    return;


pam_auth_failed:
    fprintf (stderr, "Not Authenticated.\n");

    screen_leave (au->epfd);

pam_start_failed:
    pam_end (au->pamh, res);
}


