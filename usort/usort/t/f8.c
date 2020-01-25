#define _XOPEN_SOURCE 500
#define TY double
#define TY_FMT "%20.20lf"
#include "../f8_sort.c"
#define CS f8_sort
#define ISFINITE(x) isfinite((x))
#include "ctype-cmp.c"
