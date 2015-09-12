#ifndef _INPUT_H
#define _INPUT_H

typedef struct _input_state input_state_t;

input_state_t* input_state_create (int epoll_fd);

int get_input_next_timeout (input_state_t *st);
char* get_input_state_string (input_state_t *st);
void input_state_destroy (input_state_t *st);

#endif
