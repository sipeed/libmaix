
extern "C"
{
    #include "maix_debug.h"
    #include <time.h>
    int maix_main(int argc, char *argv[])
    {
        LIBMAIX_INFO_PRINTF("maix_main %d", time(NULL));
        return 0;
    }
}