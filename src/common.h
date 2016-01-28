#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <sys/epoll.h>
#include <assert.h>

typedef struct _data_handler {
    void *data;
    int fd;
    void (*handler) (int fd, void *data);
} data_handler_t;


extern uint64_t   xget_current_time (void);
extern int   equal    (double, double);

extern int   strhash  (const char*, int, int);

extern char* xsprintf (const char *, ...);
extern char* xfgets   (FILE *);

extern char* xstrdup  (const char*);

extern void* xmalloc  (size_t);
extern void* xcalloc  (size_t, size_t);
extern void* xrealloc (void*, size_t);
extern void  xfree    (void*);

extern void _debug_log (const char *, ...);

#define debug_log(...) \
    _debug_log ("file: " __FILE__ " line: " #__LINE__ __VA_ARGS__)

#define assert_log(cond, ...) \
    if (cond) { debug_log(__VA_ARGS__); assert(0); } \
    else ((void)0)



#endif
