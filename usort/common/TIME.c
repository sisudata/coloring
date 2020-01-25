#ifndef AS_TIME
#define AS_TIME
#  ifndef AS_TIME_LKG
#    define AS_TIME_LKG inline
#  endif

#include <stdlib.h>
#include <sys/time.h>

AS_TIME_LKG double TIME() {
    struct timeval tv;
    gettimeofday(&tv,NULL); 
    return ((double)tv.tv_sec) + (1e-6 * (double)tv.tv_usec);
}

#undef AS_TIME_LKG
#endif
