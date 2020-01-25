/* c 2008 Andrew I. Schein.  All rights reserved.
   
   A fast general-purpose heap sort implementation.
   
   Caller defines a few macros:
   Required: HSORT_TY, HSORT_LT
   Recommended: 
   1. #define HS_(name) ty_##name, e.g. HS_(sort) -> u4_hsort
   To do: 
   1. handle keys within structs.
   2. define optional mode using comparison operators wrt a single CMP a la python, ocaml.
   
*/

#ifndef HSORT
#define HSORT
#include "../common/defs.c"
#ifndef CSORT_TY
#  error "hsort.c imported without HSORT_TY definition."
#endif

/* can redefine with type_ */
#ifndef CS_
#  define CS_(name) CS_##name
#endif

/* Comparisons... default to arithmatic */
#ifndef CSORT_EQ
#  define CSORT_EQ(a,b) (*(a) == *(b))
#endif
#ifndef CSORT_LT
#  define CSORT_LT(a,b) (*(a) < *(b))
#endif
#ifndef CSORT_LE
#  define CSORT_LE(a,b) (*(a) <= *(b))
#endif

static inline void CS_(siftdown)(CSORT_TY *a, const long start, const long end) {
    long child, root = start;
    while (root * 2 + 1 <= end) {
        child = root * 2 + 1;
        if ((child + 1) <= end && CSORT_LT(&a[child],&a[child+1])) ++child;           
        if (CSORT_LT(&a[root],&a[child])) CS_(csort_swap)(&a[root],&a[child]), root=child; 
        else return;      
    }
}

static inline void CS_(heapify)(CSORT_TY *a, const long count) {
    long start = (count-2) / 2; 
    while ( start >= 0 ) CS_(siftdown)(a,start--,count-1);
}

static inline void CS_(heap_sort)(CSORT_TY *a, const long count) {
    long end;
    CS_(heapify)(a, count);
    end = count - 1;
    while (end > 0) {
        CS_(csort_swap)(&a[end], &a[0]);
        CS_(siftdown)(a, 0, end-1);
        --end;   
    }
}

  #ifndef CS_KEEP
    #undef CS_
    #undef CSORT_LT
    #undef CSORT_LE
    #undef CSORT_EQ
    #undef CSORT_TY
  #endif
#endif
