#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void die(const char *message)
{
    fprintf(stderr, "%s", message);
    exit(1);
}

void *ecalloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (p == NULL)
        die("Can't allocate memory");

    return p;
}
