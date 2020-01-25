/* c 2008 Andrew I. Schein.  All rights reserved.
   
   A fast general-purpose sorting implemention based on Bentley and McIlroy, 1993's discussion
   of quicksort... altered into an introsort.
   
   Caller defines a few macros:
   Required: CSORT_TY, CSORT_LT
   Recommended: 
   1. #define CS_(name) ty_##name, e.g. CS_(sort) -> u4_qsort
   To do: 
   1. handle keys within structs.
   2. define optional mode using comparison operators wrt a single CMP a la python, ocaml.
   
   This implementation achieves a speedup compared to GLIBC 2.7 qsort by using the following
   strategies:

   1. Static definitions of comparions ensure inlining of this operation.  This is accomplished
   through C macros.
   2. Switch to insertion sort (GLIBC does this much later in the recursion tree than my impl.)
   3. Smart treatment of duplicate pivots.  Useful for arrays containing very low entropy elements.
   
   In addition, this implementation differs from GLIBC 2.7 qsort in the use of recursion.
   GLIB 2.7 manually manages a stack in order to bypass the more conventional decision of using 
   the C runtime stack.  My implementation allows the C runtime to manage the
   recursion stack for the non-tail first recursion call of quicksort, but performs the second
   recursion in tail fashion by performing a goto. 

   Something this implementation shares with the GLIBC implementation is recursing on the smaller 
   partition first.  This ensures a O(N*log(N)) bound on memory usage. Another commonality is the
   use of a smart median selection procedure the, "Tukey Ninther" method.
   
   The speedup compared to GLIBC is documented through a test harness located in the same 
   directory as this file.

*/

#include "../common/defs.c"
#include <math.h>

#ifndef CSORT
#define CSORT
#if !defined LIBBING_AS_CSORT
#  define CSORT_LKG static inline
#else
#  define CSORT_LKG 
#endif

#ifndef CSORT_TY
#  error "qsort.c imported without CSORT_TY definition."
#endif


/* can redefine with type_ */
#ifndef CS_
#  define CS_(name) CS_##name
#endif

/*
#define HSORT_TY CSORT_TY 
#define HS_(name) HS_##name
*/

#define CS_KEEP
#include "../hsort/hsort.c"  
#undef CS_KEEP
#include "swap.c" 

/* smaller than this value => insertion sort */
#define CSORT_ISORT_SWITCH 32
#define CSORT_NINTHER_SWITCH 64

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

#define CSORT_MIN(a,b) ((a) < (b) ? (a) : (b))

/* implements median of 3 "the ninther." Argumments are addresses. */
#define CSORT_NINTHER(a,b,c)                                            \
    (CSORT_LT((a),(b)) ? (CSORT_LT((b),(c)) ? (b) :                     \
                          CSORT_LT((a),(c)) ? (c) : (a))                \
     : (CSORT_LT((c),(b)) ? (b) : CSORT_LT((c),(a)) ? (c) : (a)))

static inline void CS_(intro_sort)(CSORT_TY *x, const long long orig_n, long intro_limit) {
    long long n = orig_n,s;
    CSORT_TY *p0,*pm,*p1;
    CSORT_TY *a,*b,*c,*d; /* ,*t; */ /* indices within array */
    CSORT_TY pivot;
 ssort_start:
    if (n < 0) fprintf(stderr,"sort error: n < 0: %lld\n",n),exit(1);
    if (n <= CSORT_ISORT_SWITCH) return CS_(ins_sort) (x,n); 
    if (intro_limit <= 0)        return CS_(heap_sort)(x,n);  
    s=(n>>3);
    p0=x;pm=x+(n>>1);p1=x+n-1; /* pivot candidates 0,1 from calculus, m for median */
    if (n >= CSORT_NINTHER_SWITCH) {
        p0    = CSORT_NINTHER(p0    , p0+s, p0+2*s);
        pm    = CSORT_NINTHER(pm-s  , pm  , pm+s);
        p1    = CSORT_NINTHER(p1-2*s, p1-s, p1);    
    } 
    pm    = CSORT_NINTHER(p0,pm,p1); /* now pm contains the pivot */
    pivot = *pm;
    a     = b = x;
    c     = d = x + (n-1);
    for (;;) { 
        while (b <= c && CSORT_LE(b, &pivot)) { 
            if (CSORT_EQ(b,&pivot)) CS_(csort_swap)(a++,b); /* repeated pivots treated separately */
            b++; 
        }  
        while (c >= b && CSORT_LE(&pivot,c)) { 
            if (CSORT_EQ(c,&pivot)) CS_(csort_swap)(d--,c);  
            c--; 
        }
        if (b > c) break;
        CS_(csort_swap)(b++,c--);
    }
    s = CSORT_MIN(a-x,b-a); /* repeat pivot movement */
    swap(x , b - s     , s * sizeof(CSORT_TY));
    s = CSORT_MIN(d-c, (x + n - 1) - d);
    swap(b, x + (n - s), s * sizeof(CSORT_TY));
    if ((b-a) < n-(d-c)) {  /* recurse on smaller first to bound memory usage. */
        if ((b-a) > 1) CS_(intro_sort)(x, (b-a),intro_limit-1);
        if ((n-(d-c)) > 1) { /* avoid procedure call on second recursion. */
            x = x+n-(d-c);
            n = d-c;
            intro_limit--;
            goto ssort_start;
        }
    }
    else {
        if ((d-c) > 1) CS_(intro_sort)(x + n-(d-c), d-c,intro_limit-1);
        if ((b - a) > 1) {
            n = (b-a); 
            intro_limit--;
            goto ssort_start; /* avoid procedure call on second recursion. */
        }
    }
}

static inline void CS_(sort)(CSORT_TY *x, const long long orig_n) {
    CS_(intro_sort)(x, orig_n, log(orig_n) + 3);
}

#undef CS_
#undef CSORT_MIN
#undef CSORT_LKG 
#undef CSORT_LT
#undef CSORT_LE
#undef CSORT_EQ
#undef CSORT_TY
#undef CSORT_SWITCH
#undef CSORT_NINTHER
#endif
