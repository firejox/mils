#ifndef _S_INPUT_H
#define _S_INPUT_H

#include <stddef.h>
#include <stdint.h>
#include <xkbcommon/xkbcommon-keysyms.h>

typedef struct _input_state input_state_t;

typedef uint32_t keysym_t;

input_state_t* input_state_create (int epoll_fd);

keysym_t get_input_state_info (input_state_t *st, int *timeout);

int keysym_to_utf8 (keysym_t sym, char *buf, size_t sz);

void input_state_destroy (input_state_t *st);

#endif
