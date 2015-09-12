#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <sys/epoll.h>

typedef struct _data_handler {
    void *data;
    void (*handler) (int fd, void *data);
} data_handler_t;

extern int   equal    (double, double);

extern int   strhash  (const char*, int, int);

extern char* xsprintf (const char *, ...);
extern char* xfgets   (FILE *fp);

extern char* xstrdup  (const char*);

extern void* xmalloc  (size_t);
extern void* xrealloc (void*, size_t);
extern void  xfree    (void*);

#endif
