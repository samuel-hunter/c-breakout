
#ifndef UTIL_H
#define UTIL_H

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a > b) ? b : a)

#define DEBUG_INFO  0
#define DEBUG_WARN  1
#define DEBUG_ERR   2
#define DEBUG_CRIT  3
#define DEBUG_FATAL 4

#define DEBUG_LEVEL DEBUG_INFO

extern void die(const char* fmt, ...);
extern void *ecalloc(size_t nmemb, size_t size);
extern void dbprintf(unsigned int lvl, const char* fmt, ...);

#endif // UTIL_H 
