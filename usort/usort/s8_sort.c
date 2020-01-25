/* c 2008 Andrew I. Schein */

/* reference:
 *
 * http://www.stereopsis.com/radix.html
 * (was possibly buggy wrt to prefetching)
 */


#include "s8_sort.h"
#include <string.h>

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

#include <stdlib.h>

#define CS_(name) s8_c##name
#define CSORT_TY long long
#include "../csort/csort.c"

#define _0(v) ((v)         & 0x7FF)
#define _1(v) (((v) >> 11) & 0x7FF)
#define _2(v) (((v) >> 22) & 0x7FF)
#define _3(v) (((v) >> 33) & 0x7FF)
#define _4(v) (((v) >> 44) & 0x7FF)
#define _5(v) ((((v) >> 55) & 0x7FF) ^ 0x400)

#define HIST_SIZE 2048

/* implements in place u4 radix sort. */

S8_SORT_LKG void s8_sort( long long *a, const long sz) {
    long j;
    unsigned long pos;
    long n, sum0=0,sum1=0,sum2=0,sum3=0,sum4=0,sum5=0,tsum=0;
    long long *reader, *writer, *buf = ( long long*) malloc(sz * sizeof( long long));
    if (sz < 2048) { free(buf) ; return s8_csort(a,sz); }
    if (sz < 0) { fprintf(stderr,"s8_sort: sz of array < 0: %ld\n",sz); exit(1); }
    long *b0, *b1, *b2, *b3, *b4, *b5;
    b0   = ( long*) malloc(HIST_SIZE * 6 * sizeof(long));
    b1   = b0 + HIST_SIZE;
    b2   = b1 + HIST_SIZE;
    b3   = b2 + HIST_SIZE;
    b4   = b3 + HIST_SIZE;
    b5   = b4 + HIST_SIZE;
    memset(b0,0,6 * HIST_SIZE*sizeof(long));
    for (n=0; n < sz; n++) {
        b0[_0(a[n])]++; 
        b1[_1(a[n])]++; 
        b2[_2(a[n])]++; 
        b3[_3(a[n])]++; 
        b4[_4(a[n])]++; 
        b5[_5(a[n])]++;    
    }
    for (j = 0; j < HIST_SIZE; j++) { 
        tsum  = b0[j] + sum0;
        b0[j] = sum0 - 1;
        sum0  = tsum;
        tsum  = b1[j] + sum1;
        b1[j] = sum1 - 1;
        sum1  = tsum;
        tsum  = b2[j] + sum2;
        b2[j] = sum2 - 1;
        sum2  = tsum;
        tsum  = b3[j] + sum3;
        b3[j] = sum3 - 1;
        sum3  = tsum;
        tsum  = b4[j] + sum4;
        b4[j] = sum4 - 1;
        sum4  = tsum;
        tsum  = b5[j] + sum5;
        b5[j] = sum5 - 1;
        sum5  = tsum;
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
    writer = buf;
    reader = a;
    for (n=0; n < sz; n++) {
        pos = _2(reader[n]);
        writer[++b2[pos]] = reader[n];
    }
    writer = a;
    reader = buf;
    for (n=0; n < sz; n++) {
        pos = _3(reader[n]);
        writer[++b3[pos]] = reader[n];
    }
    writer = buf;
    reader = a;
    for (n=0; n < sz; n++) {
        pos = _4(reader[n]);
        writer[++b4[pos]] = reader[n];
    }
    
    writer = a;
    reader = buf;
    for (n=0; n < sz; n++) {
        pos = _5(reader[n]);
        writer[++b5[pos]] = reader[n];
    }
    
    free(buf);  
    free(b0);
}


#undef _0
#undef _1
#undef _2
#undef HIST_SIZE

#else /* big endian */
#define CS_(name) s8_## name 
#define CSORT_TY long long
#include "../csort/csort.c"
#endif 
