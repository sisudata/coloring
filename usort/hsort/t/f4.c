#include <math.h>
#define ISNAN(x)  isnan((x))
#define TY float
#define TY_FMT "%f"
#include "../ufunc/f4_hsort.c"
#define CS f4_heap_sort
#include "hsort-cmp.c"
