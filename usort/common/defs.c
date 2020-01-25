/* c 2009 Andrew I. Schein
CSORT -- defs for comparison sorting.  Includer should undef all the macros 
*/

#ifndef CSORT_DEFS
# define CSORT_DEFS
# ifndef CS_
# error __FILE__ " included without defining CS_ macro."
# endif
# ifndef CSORT_TY
#  error __FILE__ " included without defining CSORT_TY macro."
# endif

# ifndef CSORT_EQ
#  define CSORT_EQ(a,b) (*(a) == *(b))
# endif
# ifndef CSORT_LT
#  define CSORT_LT(a,b) (*(a) < *(b))
# endif
# ifndef CSORT_LE
#  define CSORT_LE(a,b) (*(a) <= *(b))
# endif

static inline void CS_(csort_swap)(CSORT_TY *a, CSORT_TY *b) {
    /* would like to make a macro--but caller tends to increment args. */
    CSORT_TY swap = *a;
    *a            = *b;
    *b            = swap;
}

static inline void CS_(ins_sort)(CSORT_TY* a, const long long len) {    
    CSORT_TY *x=a+1,*y;
    for (;x < a + len;x++)
        for ( y=x; y>a && CSORT_LT(y,(y-1)); y-- )
            CS_(csort_swap)(y,y-1);
}

#endif /* CSORT */
