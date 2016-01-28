#ifndef __VECTOR_TEMPLATE_H__
#define __VECTOR_TEMPLATE_H__

#include "common.h"
#include <stddef.h>

#define VECTOR(TYPE) TYPE##_vector_t

#define for_each_iter_in_vector(iter, vec)  \
    for (iter = &vec->data[0]; iter != &vec->data[vec->len]; iter++)


#define VECTOR_MODULE_DEF(TYPE) \
typedef struct _##TYPE##_vector TYPE##_vector_t; \
struct _##TYPE##_vector { \
    size_t len; \
    size_t size; \
    TYPE data[]; \
}; \
static inline TYPE##_vector_t *TYPE##_vector_create(void) { \
    TYPE##_vector_t *vec = NULL; \
    size_t lim_sz = (DEFAULT_SIZE > 0) ? DEFAULT_SIZE : 1; \
    vec = xmalloc (sizeof (TYPE##_vector_t) + \
            sizeof (TYPE) * lim_sz); \
    vec->size = lim_sz; \
    vec->len = 0; \
    return vec; \
} \
static inline TYPE##_vector_add(TYPE##_vector_t *vec, TYPE item) { \
    size_t sz; \
    assert_log(vec, "this " #TYPE " vector is null\n"); \
    if (vec->len == vec->size) { \
        if (vec->size) { \
           sz = (vec->size >= SIZE_MAX/2) ? SIZE_MAX : vec->size*2; \
           vec = xrealloc (vec, sizeof(TYPE##_vector_t) + \
                   sizeof(TYPE) * sz); \
           vec->size = sz; \
        } \
    } \
    vec->arr[vec->len++] = item; \
} \
static inline void TYPE##_vector_clear(TYPE##_vector_t *vec) { \
    assert_log (vec, "this " #TYPE " vector is null\n"); \
    vec->len = 0; \
} \
static inline void TYPE##_vector_destroy(TYPE##_vector_t vec) { \
    xfree(vec); \
} \



#endif 
