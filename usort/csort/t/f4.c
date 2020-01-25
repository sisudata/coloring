#include <math.h>
#define ISNAN(x)  isnan((x))
#define TY float
#define TY_FMT "%f"
#include "../ufunc/f4_sort.c"
#define CS f4_sort
#include "csort-cmp.c"
