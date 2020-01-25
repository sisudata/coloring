/* c 2008 Andrew I. Schein */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "u1_sort.h"
#define U1_HIST_SIZE 256

#define CSORT_TY unsigned char
#define CS_(name) u1_##name
#include "../common/defs.c"

/* implements in place u1 bucket sort. */

U1_SORT_LKG void u1_sort(unsigned char *a, const long sz) {
    long j;
    long n;
    unsigned char *writer=a; 
    long b0[U1_HIST_SIZE];
    if (sz < 32) { return CS_(ins_sort)(a,sz);}
    memset(b0,0,U1_HIST_SIZE * sizeof(long));
    for (n=0; n < sz; n++) {
        b0[a[n]]++; 
    }
    
    for (j = 0; j < U1_HIST_SIZE; j++) { 
        while (b0[j]-- > 0) { 
            *writer = j; 
            writer++; 
        }
    }
}

#undef REFRESH
#undef U1_HIST_SIZE
#undef CSORT_TY 
#undef CS_
