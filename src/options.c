#include "common.h"
#include "options.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>

#define BUCKETS_NUM 1000
#define BUCKETS_BASE 60

typedef struct _option {
    char         *desc;/*option description*/
    option_data_t data;/**option data*/
    option_type_t type;/*option type*/
    int           flag;/*rewrite type by file*/

    struct _option *hash_next;
    struct _option *link_next;
} option_t;

struct _options {
    option_t *buckets[BUCKETS_NUM];
    option_t *head;
};


static void options_add_option (options_t *opts,
        const char* desc, option_type_t type, int flag, ...) {
    va_list ap;
    option_t *op;
    int hash_val;

    fprintf (stderr, "malloc %s \n", desc);
    op = xmalloc (sizeof (struct _option));

    op->desc = xstrdup (desc);
    op->type = type;
    op->flag = flag;
    
    va_start (ap, flag);
    switch (type) {
        case OPTION_RGB:
            op->data.color.r = va_arg (ap, double);
            op->data.color.g = va_arg (ap, double);
            op->data.color.b = va_arg (ap, double);
            break;
        
        case OPTION_STRING:
            op->data.str = xstrdup (va_arg (ap, char*));
            break;
        
        case OPTION_INT:
            op->data.num = va_arg (ap, uint32_t);
            break;

        case OPTION_DOUBLE:
            op->data.fp = va_arg (ap, double);
            break;

        default :
            fprintf (stderr, "Invalid option!!\n");
            exit (-1);
    
    }
    va_end (ap);

    hash_val = strhash (desc, BUCKETS_BASE, BUCKETS_NUM);

    op->hash_next = opts->buckets[hash_val];
    op->link_next = opts->head;

    opts->buckets[hash_val] = op;
    opts->head = op;

}

static void options_default (options_t *opts) {
    options_add_option (opts,
            "background_color", OPTION_RGB,
            0, 0.0, 0.0, 0.0);

    /*input panel option*/
    options_add_option (opts,
            "input_panel_x", OPTION_INT, 
            1, 0);
    options_add_option (opts,
            "input_panel_y", OPTION_INT,
            1, 0);
    
    /*input option*/
    options_add_option (opts, 
            "input_name_x", OPTION_INT, 1, 0);
    options_add_option (opts,
            "input_name_y", OPTION_INT, 1, 0);
    options_add_option (opts,
            "input_pass_x", OPTION_INT, 1, -1);
    options_add_option (opts,
            "input_pass_y", OPTION_INT, 1, -1);

    options_add_option (opts, 
            "input_color", OPTION_RGB,
            0, 0.0, 0.0, 0.0);
    options_add_option (opts, 
            "input_font", OPTION_STRING,
            0, "Verdana Bold 12");

    options_add_option (opts,
            "input_maxlength_name", OPTION_INT,
            0, 20);
    options_add_option (opts,
            "input_maxlength_pass", OPTION_INT, 
            0, 20);

    /*username option*/
    options_add_option (opts,
            "username_x", OPTION_INT, 1, -1);
    options_add_option (opts, 
            "username_y", OPTION_INT, 1, -1);
    
    options_add_option (opts,
            "username_color", OPTION_RGB,
            0, 0.0, 0.0, 0.0);
    options_add_option (opts,
            "username_font", OPTION_STRING,
            0, "Verdana Bold 14");
    options_add_option (opts, 
            "username_msg", OPTION_STRING,
            0, "username:");

    /*password option*/
    options_add_option (opts, 
            "password_x", OPTION_INT, 1, -1);
    options_add_option (opts, 
            "password_y", OPTION_INT, 1, -1);

    options_add_option (opts, 
            "password_color", OPTION_RGB,
            0, 0.0, 0.0, 0.0);
    options_add_option (opts,
            "password_font", OPTION_STRING,
            0, "Verdana Bold 14");
    options_add_option (opts,
            "password_msg", OPTION_STRING,
            0, "password:");
}

static option_t* find_option (options_t *opts, const char *desc) {
    int hash_val = strhash (desc, BUCKETS_BASE, BUCKETS_NUM);
    option_t *p;

    for (p = opts->buckets[hash_val]; p; p = p->hash_next) 
        if (!strcmp (p->desc, desc))
            return p;
    return NULL;
}

static void option_setup_rgb 
        (option_t *op, char *data) {
    uint8_t r, g, b;
    sscanf (data, "#%2"SCNx8"%2"SCNx8"%2"SCNx8, 
            &r, &g, &b);
    op->data.color.r = (double)r/255.0;
    op->data.color.g = (double)g/255.0;
    op->data.color.b = (double)b/255.0;
}

static void option_setup_string
        (option_t *op, char *data) {
    size_t sz = strlen (data);
    while (sz && isblank (data[sz - 1]))
        sz--;

    op->data.str = xrealloc (op->data.str, sz + 1);
    strncpy (op->data.str, data, sz);
    op->data.str[sz] = '\0';
}

static void option_setup_int 
        (option_t *op, char *data) {
    uint32_t tmp;
    char *p;

    if (op->flag && (p = strchr (data, '%'))) {
        *p = '\0';
        sscanf (data, "%"SCNu32, &tmp);
        op->data.fp = (double)tmp / 100.0;
        op->type = OPTION_DOUBLE;
    }
    else
        sscanf (data, "%"SCNu32, &op->data.num);
}

static void option_setup_double
        (option_t *op, char *data) {
    uint32_t tmp;
    char *p;

    if (op->flag && !(p = strchr (data, '%'))) { 
        sscanf (data, "%"SCNu32, &op->data.num);
        op->type = OPTION_INT;
    }
    else {
        *p = '\0';
        sscanf (data, "%"SCNu32, &tmp);
        op->data.fp = (double)tmp / 100.0;
    }
}


static void (*op_parse[4]) (option_t*, char*) = {
    option_setup_rgb,
    option_setup_string,
    option_setup_int,
    option_setup_double
};

options_t* options_create (const char *path) {
    int i;
    options_t *opts = xmalloc (sizeof (options_t));
    option_t *op;
    FILE *fp = NULL;
    char *line, *ptr;
    size_t sz;

    opts->head = NULL;
    for (i = 0; i < BUCKETS_NUM; i++)
        opts->buckets[i] = NULL;

    options_default (opts);

    fp = fopen (path, "r");
    if (!fp) {
        fprintf (stderr, "open file failed: %s",
                strerror (errno));
        return opts;
    }
    
    while ((line = xfgets (fp))) {
        ptr = strtok (line, " \t\r\n");
        if (!ptr || (*ptr == '\0'||*ptr == '#'))
            goto free_line;
    
        op = find_option (opts, ptr);
        
        ptr = strtok (NULL, "");
        sz = strspn (ptr, " \t");
        ptr = ptr + strspn (ptr, " \t");

        if (op) 
            op_parse[op->type] (op, ptr);
        
free_line :
        xfree (line);
    }

    fclose (fp);

    return opts;
}

const option_data_t* get_option_data 
        (options_t *opts, const char *desc,
         option_type_t *type) {
    option_t *op = NULL;
    op = find_option (opts, desc);
    if (op) 
        *type = op->type;
    return &(op->data);
}

void options_destroy (options_t *opts) {
    option_t *p, *q;
    p = opts->head;
    while (p) {
        q = p;
        p = p->link_next;
        
        xfree (q->desc);
        if (q->type == OPTION_STRING)
            xfree (q->data.str);
        
        xfree (q);
    }

    xfree (opts);
}
