#include <errno.h>

#define LOG_ERR(...)    {printf(__VA_ARGS__); printf("\n");}

#define ARR_SIZE(a)     (sizeof(a)/sizeof(a[0]))
