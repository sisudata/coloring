/* Test harness for c type specific sorts, useful for all C base types up to 64 bits.  
   Uses macros for polymorphism.
   To use, define macro TY (e.g. #define TY float) then #include this file. */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "common/TIME.c"
#include <assert.h>
#include <unistd.h>
#ifndef ISFINITE
#define ISFINITE(x) 1
#endif

enum generator {RAND, BOUNDED, SORTED, REVERSE ,IDENT} ;
union { char l[8]; TY t; double d; unsigned long long llu; } u, u1, u2;
    

long long k,j;
const char* usage="csort-cmp N dist trials\n"
"dist is one of: RAND, BOUNDED, SORTED, REVERSE, IDENT\n"
"N:              size of the array.\n"
"trials:         how many trials to do.  Necessary for small N.\n";

int parseDist(char* dist_str) {
    if (!strcmp("BOUNDED",dist_str)) 
        return BOUNDED;
    else if (!strcmp("RAND",dist_str)) 
        return RAND;
    else if (!strcmp("SORTED",dist_str))
        return SORTED;
    else if (!strcmp("REVERSE",dist_str))
        return REVERSE;
    else if (!strcmp("IDENT",dist_str))
        return IDENT;
    else fprintf(stderr,"dist argument mismatch.\n%s\n",usage),exit(1);
}

void randomized(TY *x, long long n) {
    long long i; 
    for (i = 0; i < n; i++) {
    repeat :
        u.l[0] =  random();
        u.l[1] =  random();
        u.l[2] =  random();
        u.l[3] =  random();
	u.l[4] =  random();
        u.l[5] =  random();
        u.l[6] =  random();
        u.l[7] =  random();
        x[i]   =  u.t; 
        if (!ISFINITE(x[i])) {goto repeat;}
        assert(x[i] == x[i]);
        //fprintf(stderr,"found: %llu\n",u.llu);
    }
}

void bounded(TY *x, long long n) {
    long long i;
    union { long l[2]; TY t;} u;
    for (i = 0; i < n; i++) {
        u.l[0] = (random() % (n/4));
        u.l[1] = (random() % (n/4));
        x[i]   = u.t;
        if (!ISFINITE(x[i])) x[i] = 0;
    }
}

void identity(TY *x, long long n) {
    long long i;
    for (i = 0; i < n; i++) {
        x[i] = 1;
    }
}

void reverse(TY *x, long long n) {
    long long i;
    for (i = 0; i < n; i++) 
        x[i] = n - i - 1;
}

void sorted(TY *x, long long n) {
    long long i;
    for (i = 0; i < n; i++) 
        x[i] = i;
}

int compare(const void *a, const void *b) {
    TY A = *(const TY *)a, B = *(const TY *)b;
    /* fprintf(stderr, "compare(a, b): [  %d  -vs-  %d  ]\n", *(TY *) a, *(TY *) b); */
    if (A > B) { return 1; }
    if (B > A) { return -1; }
    return 0;
}

void fill(char* dist, TY* array1,long n) {
    switch (parseDist(dist)) {
    case RAND : 
        randomized(array1,n) ; break;
    case BOUNDED :
        bounded(array1,n) ;    break;
    case SORTED :
        sorted(array1,n)  ;    break;
    case REVERSE :
        reverse(array1,n) ;    break;
    case IDENT :
        identity(array1,n) ;   break;
    default :
        fprintf(stderr,"dist match error.\n"), exit(1);
    }
}

void cmpWork(const TY *g, const TY *m, long long n ) {
    long long i;
    for (i = 1; i < n; i++) {
        u1.llu=0; u2.llu=0;
        u1.d = g[i]; u2.d = g[i];
        
        if (u1.llu != u2.llu) {
            fprintf (stderr,"cmpWork: %lld x %zd: failure at offset %lld " TY_FMT " " TY_FMT  "\n",
                     n, sizeof(TY), i, m[i] , g[i] );
            exit(1);
        }
    }
}
void checkWork(const char *name, TY *a, long long n ) {
    long long i,j;
    for (i = 1; i < n; i++) {
        if (a[i-1] > a[i]) {
            fprintf (stderr,"%s: %lld x %zd: failure at offset %lld\n", name, n,
                     sizeof(TY), i);
            for (j=0; j < n;j++)
                if (j != i-1) u.d=a[k],fprintf(stderr, TY_FMT " ", a[j]);
                else fprintf(stderr, "*" TY_FMT "* ", a[j]);
                                  
            
            fprintf(stderr,"\n"); 
            free(a);
            exit(1);
        }
    }
}

int main (int argc, char **argv)
{
    if (argc < 4) fprintf(stderr,"too few arguments: %d\n%s",argc,usage) , exit(1);
    long i=j=0; long n=strtoul(argv[1],NULL,10);
    long num_trials = strtoul(argv[3],NULL,10);
    double start, end, m_tot=0, g_tot=0;
    TY *array_orig = (TY*) malloc (n * sizeof(TY));
    TY *array_g = (TY*) malloc (n * sizeof(TY));
    TY *array_m = (TY*) malloc (n * sizeof(TY));
    srandom(TIME());
    if (array_orig == NULL)
        {
            fprintf (stderr,"%d x %zd: no memory\n", argc, sizeof(TY));
            return 1;
        }
    if (getenv("SEED")) srand(time(NULL)); /* default is debugging mode. */
    for (i = 0; i < num_trials; i++) {
        fill(argv[2], array_orig, n);
        memcpy(array_g, array_orig, n*sizeof(TY));
        memcpy(array_m, array_orig, n*sizeof(TY));
        
        start = TIME();
        qsort(array_g, n, sizeof(TY), &compare);
        end = TIME();

        if (i) {
            g_tot += end - start;
        }  
        checkWork("GNU", array_g, n);
        u1.d = array_g[0] ; u2.d = array_g[1];
        //fprintf(stderr,"GNU: %llx %llx\n",u1.ull,u2.ull);
        
        start = TIME();
        CS(array_m,n);
        end   = TIME();
        if (i) {
            m_tot += end - start;
        }    
        u1.d = array_m[0]; u2.d = array_m[1];
        //fprintf(stderr,"schein: %llx %llx\n",u1.ull,u2.ull);
        
        checkWork("schein",array_m,n);
        cmpWork(array_g,array_m,n);
        
        
    }
    g_tot /= (double) (num_trials - 1);
    m_tot /= (double) (num_trials - 1);
    fprintf(stdout,"%10ld      %5.10f    %5.10f          %2.2f\n",n,m_tot,g_tot,(g_tot/m_tot));
    free (array_m);
    free (array_g);
    free (array_orig);
    return 0; 
}





