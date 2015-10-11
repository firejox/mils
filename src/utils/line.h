#ifndef _S_LINE_TEXT_H
#define _S_LINE_TEXT_H

#include <stddef.h>
#include <stdint.h>

#define LINE_TEXT_EVENT_NUM 4

typedef enum {
    CURSOR_MOVE_LEFT,
    CURSOR_MOVE_RIGHT,
    APPEND_CH,
    DELETE_LEFT_CH,
    DELETE_RIGHT_CH
} line_text_event_type;

typedef struct _line_text line_text_t;

line_text_t* line_text_create (size_t lim);

line_text_t* line_text_ref (line_text_t *lt);

void line_text_handle_event (line_text_t *lt,
        line_text_event_type evt, ...);


char* line_text_to_string (line_text_t *lt);

int line_text_cursor_pos (line_text_t *lt);

void line_text_unref (line_text_t *lt);


#endif
