#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"

#include "libmaix_nn.h"
#include "libmaix_nn_decoder_retinaface.h"

#include "hello.h"

void hello()
{
    printf("hello libmaix\n");
}

int main(int argc, char* argv[])
{
    hello();
    return 0;
}

