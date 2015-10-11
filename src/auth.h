#ifndef _AUTH_H
#define _AUTH_H

typedef struct _auth auth_t;


auth_t* auth_create(void);

void    auth_loop (auth_t *auth);
void    auth_once (auth_t *auth);

void    auth_destory(auth_t *auth);



#endif
