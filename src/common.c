#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>


uint64_t xget_current_time (void) {
    struct timespec tm;
    uint64_t calc;
    clock_gettime (CLOCK_MONOTONIC, &tm);
    calc = tm.tv_sec * UINT64_C(1000) + tm.tv_nsec / UINT64_C(1000000);
//    fprintf (stderr, "%"PRIu64"\n", calc);
    return calc;
}

int equal (double a, double b) {
    return fabs (a - b) < 1e-5;
}

int strhash (const char *str, int base, int mod) {
    int ans = 0, i;
    base %= mod;

    for (i = 0; str[i]; i++) 
        ans = (ans*base + (int)str[i]) % mod;
    return ans;
} 

char* xsprintf (const char *fmt, ...) {
    int len;
    char*  str;
    va_list ap;

    va_start (ap, fmt);
    len = vsnprintf (NULL, 0, fmt, ap);
    va_end (ap);

    if (len < 0) {
        fprintf (stderr, "there is error for"
                " calculating format length\n");
        exit (-1);
    }

    len++;

    str = xmalloc (len);

    va_start(ap, fmt);
    len = vsnprintf (str, len, fmt, ap);
    va_end(ap);
    
    if (len < 0) {
        fprintf (stderr, "there is error for"
                " making string.\n");
        exit (-1);
    }

    return str;
}


static struct {
    char *buf;
    size_t cur;
    size_t sz;
} line_buf;

#define EXTEND_SZ(sz) ((sz) >= (SIZE_MAX/2) ? SIZE_MAX:\
        ((sz)*2))

char* xfgets (FILE *fp)  {
    static int init_flag = 1;
    char *tmp = NULL;
    size_t len;

    if (init_flag) {
        line_buf.buf = xmalloc (100);
        line_buf.cur = 0;
        line_buf.sz  = 100;

        init_flag = 0;
    }
    
    while (fgets (line_buf.buf + line_buf.cur,
                line_buf.sz - line_buf.cur, fp) != NULL) {
        len = strlen (line_buf.buf + line_buf.cur);
        line_buf.cur += len;

        if (line_buf.buf[line_buf.cur - 1] != '\n') {
            if ((line_buf.sz - line_buf.cur) == 1) {
                line_buf.sz = EXTEND_SZ(line_buf.sz);
                line_buf.buf = xrealloc (line_buf.buf, line_buf.sz);
            }
        } else {
            line_buf.cur = 0;
            return xstrdup (line_buf.buf);
        }
        
    }

    return NULL;
}

char* xstrdup (const char *str) {
    char *new_str = NULL;
    new_str = strdup (str);
    if (new_str == NULL) {
        fprintf (stderr, "there is no free"
                " memory for duplicating string: "
                "%s\n", strerror (errno));
        exit (EXIT_FAILURE);
    }
    return new_str;
}



void* xmalloc (size_t sz) {
    void *mem = NULL;
    mem = malloc (sz);
    if (mem == NULL) {
        fprintf (stderr, 
                "there is no free memory: %s!\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    return mem;
}

void* xcalloc (size_t nmemb, size_t sz) {
    void *mem = NULL;
    mem = calloc (nmemb, sz);
    if (mem == NULL) {
        fprintf (stderr, 
                "there is no free memory: %s!\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    return mem;
}

void* xrealloc (void *ptr, size_t sz) {
    void *mem = NULL;
    
    mem = realloc (ptr, sz);
    
    if (mem == NULL) {
        fprintf (stderr, 
                "there is no free memory: %s!\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    
    return mem;
}

void xfree (void *mem) {
    if (mem)
        free (mem);
}
