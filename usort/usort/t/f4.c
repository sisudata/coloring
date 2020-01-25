#define _BSD_SOURCE 1
#define _POSIX_C_SOURCE 200112L
#include <math.h>
#define TY float
#define TY_FMT "%f"
#include "../f4_sort.c"
#define CS f4_sort
#define ISFINITE(x) isfinite((x))
#include "ctype-cmp.c"
