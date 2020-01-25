#include <math.h>
#define ISNAN(x) isnan((x)) 
#define TY double
#define TY_FMT "%lf"
#include "../ufunc/f8_hsort.c"
#define CS f8_heap_sort
#include "hsort-cmp.c"
