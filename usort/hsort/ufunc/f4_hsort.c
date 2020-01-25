/* the <= and == tests for floats are extremely slow for certain distributions of numbers
exhibited in the test harness.  So we turn off sections of the sorting alg that use this
with CSORT_SKIP_EQUALITY */

#ifndef F4_CSORT
#define F4_CSORT
#define CSORT_TY float
#define CS_(name) f4_## name

#include "../hsort.c"
#endif
