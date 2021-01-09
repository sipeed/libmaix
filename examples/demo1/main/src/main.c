
#include "stdio.h"
#include "test.h"
#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"
#include "libmaix_nn.h"


#include "libmaix.h"


int main()
{
    printf("hello\n");
    test_maix();
    libmaix_nn_module_init();
    printf("end\n");
    return 0;
}

