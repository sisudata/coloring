/* c 2009 Andrew I. Schein */

/* reference:
 *
 * http://www.stereopsis.com/radix.html
 * (was possibly buggy wrt to prefetching)
 */

#include "f8_sort.h"
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

#  include <stdlib.h>
#  define CSORT_TY double
#  define CS_(name) f8_c##name
#  include "../csort/csort.c"

#define _0(v) ( (v)         & 0x7FF)
#define _1(v) (((v)  >> 11) & 0x7FF)
#define _2(v) (((v)  >> 22) & 0x7FF)
#define _3(v) (((v)  >> 33) & 0x7FF)
#define _4(v) (((v)  >> 44) & 0x7FF)
#define _5(v) (((v)  >> 55) & 0x7FF)
#define F8_SORT_HIST_SIZE 2048

static inline unsigned long long f8_sort_FloatFlip(unsigned long long u) {
    unsigned long long mask       =  -(u >> 63) | 0x8000000000000000ull ;
    return                  (u ^ mask);
} 

static inline unsigned long long f8_sort_IFloatFlip(unsigned long long u) {
    unsigned long long mask       =  ((u >> 63) - 1) | 0x8000000000000000ull;
    return                  (u ^ mask);
}

F8_SORT_LKG void f8_sort(double *a, const long sz) {
    long j;
    unsigned long pos;
    long n, sum0=0 , sum1=0 , sum2=0, sum3=0, sum4=0, sum5=0, tsum=0;
    unsigned long long *reader, *writer, *buf1 = (unsigned long long*) a, *buf2;
    unsigned long *b0, *b1, *b2, *b3, *b4, *b5;

    if (sz < 2048) return f8_csort(a,sz);  
    buf2  = (unsigned long long*) malloc(sz * sizeof(double));
    b0   = calloc(F8_SORT_HIST_SIZE * 6 , sizeof(unsigned long));
    b1   = b0 + F8_SORT_HIST_SIZE;    b2   = b1 + F8_SORT_HIST_SIZE;
    b3   = b2 + F8_SORT_HIST_SIZE;    b4   = b3 + F8_SORT_HIST_SIZE;
    b5   = b4 + F8_SORT_HIST_SIZE;
        
    for (n=0; n < sz; n++) {
        buf1[n] = f8_sort_FloatFlip(buf1[n]);
        b0[_0(buf1[n])]++;  b1[_1(buf1[n])]++; 
        b2[_2(buf1[n])]++;  b3[_3(buf1[n])]++; 
        b4[_4(buf1[n])]++;  b5[_5(buf1[n])]++; 
    }
    
    for (j = 0; j < F8_SORT_HIST_SIZE; j++) { 
        
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
        sum5 = tsum;
      
    }   
    
    
    writer = buf2;  /* byte 0 */
    reader = buf1;
    for (n=0; n < sz; n++) {
        pos = _0(reader[n]);
        writer[++b0[pos]] = reader[n];
        
    }
    
    
    writer = buf1;    /* byte 1 */
    reader = buf2;
    for (n=0; n < sz; n++) {
        pos = _1(reader[n]); 
        writer[++b1[pos]] = reader[n] ;
    }
    
    writer = buf2;    /* byte 2 */
    reader = buf1;
    for (n=0; n < sz; n++) {
        pos = _2(reader[n]); 
        writer[++b2[pos]] = reader[n];
        
    }
    
    writer = buf1;    /* byte 3 */
    reader = buf2;
    for (n=0; n < sz; n++) {
        pos = _3(reader[n]); 
        writer[++b3[pos]] = reader[n];
        
    }
   
    
    writer = buf2;    /* byte 4 */
    reader = buf1;
    
    for (n=0; n < sz; n++) {
        pos = _4(reader[n]); 
        writer[++b4[pos]] = reader[n];
    }
        
    writer = buf1;  /* byte 5 */
    reader = buf2;
    for (n=0; n < sz; n++) {
        pos = _5(reader[n]);
        writer[++b5[pos]] = f8_sort_IFloatFlip(reader[n]);
    }

    free(buf2);  
    free(b0);
}

# undef F8_SORT_HIST_SIZE
# undef _0
# undef _1
# undef _2
# undef _3
# undef _4
# undef _5
#else /* endian */
# define CS_(name) f8_## name 
# define CSORT_TY double
# include "../csort/csort.c"
#endif
