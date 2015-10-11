#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistr.h>
#include <uniconv.h>
#include "line.h"
#include "common.h"

typedef struct _char_node {
    uint32_t ch;
    struct _char_node *next;
    struct _char_node *prev;
} char_node_t;

struct _line_text {
    char_node_t *str;
    char_node_t *cur;
    size_t lim;
    size_t len;
    int ref_count;
    int cur_pos;
};


static char_node_t* char_node_create (uint32_t ch) {
    char_node_t *node = xmalloc (sizeof (char_node_t));
    node->ch = ch;
    node->prev = node->right = NULL;
    return node;
}

line_text_t* line_text_create (size_t lim) {
    line_text_t *line = xmalloc (sizeof (line_text_t));

    line->str = NULL;
    line->cur = NULL;
    line->lim = lim;
    line->len = 0;
    line->cur_pos = 0;
    line->ref_count = 0;

    return line;
}

line_text_t* line_text_ref (line_text_t *lt) {
    if (lt)
        lt->ref_count++;
    return lt;
}

static void cursor_left (line_text_t *lt, va_list vl) {
    if (lt->cur_pos) {
        if (lt->cur_pos > 1)
            lt->cur = lt->cur->prev;
        lt->cur_pos--;
    }
}

static void cursor_right (line_text_t *lt, va_list vl) {
    if (lt->cur_pos < lt->len) {
        if (lt->cur_pos)
            lt->cur = lt->cur->next;
        lt->cur_pos++;
    }
}

static void append_ch (line_text_t *lt, va_list vl) {
    uint32_t code = va_arg (vl, uint32_t);
    char_node_t *tmp;

    if (lt->cur == NULL) {
        lt->str = lt->cur = char_node_create (code);
        lt->cur_pos ++;
        lt->len ++;
    } else if (lt->len < lt->lim) {
        tmp = char_node_create (node);

        if (lt->cur_pos) {
            tmp->next = lt->cur->next;
            tmp->prev = lt->cur;
        
            if (lt->cur->next) 
                lt->cur->next->prev = tmp;
            lt->cur->next = tmp;

            lt->cur = lt->cur->next;

        } else {
            tmp->next = lt->cur;
            lt->cur->prev = tmp;
            lt->cur = lt->str = tmp;
        }

        lt->cur_pos ++;
        lt->len ++;
    }

}

static void delete_left_ch (line_text_t *lt, va_list vl) {
    char_node_t *tmp;

    if (lt->cur_pos) {
        tmp = lt->cur;
        if (tmp->prev) 
            tmp->prev->next = tmp->next;

        if (tmp->next)
            tmp->next->prev = tmp->prev;

        if (tmp->prev)
            lt->cur = tmp->prev;
        else
            lt->cur = lt->str = tmp->next;
        
        lt->cur_pos --;
        lt->len --;

        if (!lt->len) 
            lt->cur = lt->str = NULL;
        

        xfree (tmp);
    }
}

static void delete_right_ch (line_text_t *lt, va_list vl) {
    char_node_t *tmp;

    if (lt->cur_pos < lt->len) {

        if (lt->cur_pos) {
            tmp = lt->cur->next;
            lt->cur->next = tmp->next;
            
            if (tmp->next) 
                tmp->next->prev = lt->cur;

        } else {
            tmp = lt->cur;
            lt->str = lt->cur = tmp->next;

            if (tmp->next)
                tmp->next->prev = NULL;
        }

        lt->len--;
        if (!lt->len) 
            lt->cur = lt->str = NULL;

        xfree (tmp);
    }
}

static void (*lt_handle_ev[LINE_TEXT_EVENT_NUM])
    (line_text_t *lt, va_list vl) = {
    cursor_left,
    cursor_right,
    append_ch,
    delete_left_ch,
    delete_right_ch
};

void line_text_handle_event (line_text_t *lt,
        line_text_event_type evt, ...) {
    va_list vl;
    va_start (vl, evt);
    lt_handle_ev[evt](lt, vl);
    va_end (vl);
}

char* line_text_to_string (line_text_t *lt) {
    char_node_t *p;
    uint32_t *u32str = xmalloc ((lt->len + 1) * sizeof (uint32_t));
    int i;
    u32str[lt->len] = 0;
    char *str = NULL;
    
    for (p = lt->str, i = 0; p; p = p->next, i++)
        u32str[i] = p->ch;
    
    str = u32_strconv_to_locale (u32str);
    if (str == NULL) {
        fprintf (stderr, "convert line text failed: %s\n",
                strerror (errno));
        exit (-1);
    }

    xfree (u32str);

    return str;
}

int line_text_cursor_pos (line_text_t *lt) {
    return lt->cur_pos;
}

void line_text_unref (line_text_t *lt) {
    if (lt->ref_count)
        lt->ref_count--;
    else
        xfree (lt);
}
