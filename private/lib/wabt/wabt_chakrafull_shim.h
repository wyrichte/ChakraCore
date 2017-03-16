#define va_copy(destination, source) ((destination) = (source))
//#define snprintf _snprintf_c
#define strtof (float)strtod

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL   ((double)INFINITY)
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
#ifndef _HUGE_ENUF
    #define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif
