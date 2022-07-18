
#define DISPLAY_TIME 0

#if DISPLAY_TIME
    #include "stdio.h"
    #include <sys/time.h>
    struct timeval start, end;
    int64_t interval_s;
    #define CALC_TIME_START() do{gettimeofday( &start, NULL );}while(0)
    #define CALC_TIME_END(name)   do{gettimeofday( &end, NULL ); \
                                interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll; \
                                printf("%s use time: %lld us\n", name, interval_s + end.tv_usec - start.tv_usec);\
            }while(0)
#else
    #define CALC_TIME_START()
    #define CALC_TIME_END(name)
#endif

