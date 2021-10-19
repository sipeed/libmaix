#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"

#include "libmaix_disp.h"
#include "libmaix_image.h"
#include "libmaix_cv_image.h"

#include <sys/time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    libmaix_cv_image_test();

    libmaix_image_module_init();
    struct libmaix_disp *disp = libmaix_disp_create(0);
    if (disp)
    {
        libmaix_image_t *rgb888 = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        if (rgb888)
        {
            printf("w %d h %d p %d \r\n", rgb888->width, rgb888->height, rgb888->mode);

            disp->draw_image(disp, rgb888);

            libmaix_image_destroy(&rgb888);
        }
        libmaix_disp_destroy(&disp);
    }
    libmaix_image_module_deinit();
    return 0;
}
