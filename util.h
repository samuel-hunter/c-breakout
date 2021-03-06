
#ifndef UTIL_H
#define UTIL_H

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a > b) ? b : a)
#define WITHIN(x, min, max) ((x >= min) && (x <= max))
#define ABS(x) ((x >= 0) ? (x) : -(x))
#define SIGNUM(x) ((x >= 0) ? 1 : -1)
#define CLAMP(x, min, max) ((x > max) ? (max) : ((x < min) ? (min) : (x)))
#define LEN(a)    (sizeof(a) / sizeof(a[0]))

#define DEBUG_WARN   0x01
#define DEBUG_ERR    0x02

#define DEBUG_GAME   0x04
#define DEBUG_BALL   0x08
#define DEBUG_BRICK  0x10

#define DEBUG_LEVEL (DEBUG_WARN | DEBUG_ERR)

extern void die(const char* fmt, ...);
extern void *ecalloc(size_t nmemb, size_t size);
extern void dbprintf(unsigned int lvl, const char* fmt, ...);

#endif // UTIL_H 
