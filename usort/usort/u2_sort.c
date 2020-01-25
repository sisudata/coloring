/* c 2008 Andrew I. Schein */

/* reference:
 *
 * http://www.stereopsis.com/radix.html
 * (was possibly buggy wrt to prefetching)
 */

#include "u2_sort.h"
#include <string.h>  /* __BYTE_ORDER */

#ifndef __BYTE_ORDER
# ifdef __APPLE__
#  define __BYTE_ORDER __DARWIN_BYTE_ORDER
#  define __LITTLE_ENDIAN __DARWIN_LITTLE_ENDIAN
#  define __BIG_ENDIAN __DARWIN_BIG_ENDIAN
# else
#  error __FILE__ ": __BYTE_ORDER is not defined!"
# endif
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#include <stdlib.h> /* malloc among others */

#define CSORT_TY unsigned short
#define CS_(name) u2_##name
#include "../common/defs.c"

#define _0(v) ((v)         & 0xFF)
#define _1(v) (((v) >> 8)  & 0xFF)
#define HIST_SIZE 256

/* implements in place u4 radix sort. */

U2_SORT_LKG void u2_sort(unsigned short *a, const long sz) {
    long j;
    unsigned pos;
    long n, sum0=0 , sum1=0 , tsum=0;
    unsigned short *reader, *writer, *buf;
    long *b0, *b1;
    if (sz < 16) { return CS_(ins_sort)(a,sz); }
    buf = (unsigned short*) malloc(sz * sizeof(unsigned short));
    b0   = calloc(HIST_SIZE * 2, sizeof(long));
    b1   = b0 + HIST_SIZE;
    
    for (n=0; n < sz; n++) {
        b0[_0(a[n])]++; 
        b1[_1(a[n])]++; 
    }
    for (j = 0; j < HIST_SIZE; j++) { 
        tsum  = b0[j] + sum0;
        b0[j] = sum0 - 1;
        sum0  = tsum;
        
        tsum  = b1[j] + sum1;
        b1[j] = sum1 - 1;
        sum1  = tsum;
    }   
    writer = buf;
    reader = a;
    for (n=0; n < sz; n++) {
        pos = _0(reader[n]);
        writer[++b0[pos]] = reader[n];
    }
    writer = a;
    reader = buf;
    for (n=0; n < sz; n++) {
        pos = _1(reader[n]); 
        writer[++b1[pos]] = reader[n];
    }
    free(buf);  
    free(b0);
}

#undef _0
#undef _1
#undef HIST_SIZE
#undef CS_
#undef CSORT_TY

/* endian */
#else
#define CS_(name) u2_## name 
#define CSORT_TY unsigned short
#include "../csort/csort.c"
#endif
