#include <math.h>
#define ISNAN(x) isnan((x))
#define TY double
#define TY_FMT "%lf"
#include "../ufunc/f8_sort.c"
#define CS f8_sort
#include "csort-cmp.c"
