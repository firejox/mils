#ifndef _AUTH_H
#define _AUTH_H

typedef struct _auth auth_t;
typedef struct _auth_shm auth_shm_t; 


auth_t* auth_create();


void    auth_destory();

auth_shm_t* auth_get_shm   (auth_t* auth);
auth_shm_t* auth_shm_ref   (auth_shm_t *buf);
void        auth_shm_unref (auth_shm_t *buf);


#endif
