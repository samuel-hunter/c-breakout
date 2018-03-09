
#ifndef UTIL_H
#define UTIL_H

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a > b) ? b : a)

extern void die(const char*);
extern void *ecalloc(size_t nmemb, size_t size);

#endif // UTIL_H 
