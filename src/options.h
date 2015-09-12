#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct _rgb {
    double r;
    double g;
    double b;
} rgb_t;

typedef enum {
    OPTION_RGB,
    OPTION_STRING,
    OPTION_INT,
    OPTION_DOUBLE
} option_type_t;

typedef union _option_data {
    char *str;
    uint32_t num;
    double   fp;
    rgb_t color;
} option_data_t;

typedef struct _options options_t;

options_t* options_create (const char *path);

const option_data_t* get_option_data (options_t *opts, 
        const char* desc, option_type_t *type);

void options_destroy (options_t* opts);

#endif
