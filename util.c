#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    
    exit(1);
}

void *ecalloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (p == NULL)
        die("Can't allocate memory");

    return p;
}

void dbprintf(unsigned int lvl, const char *fmt, ...)
{
    if (lvl < DEBUG_LEVEL) return;
    
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
