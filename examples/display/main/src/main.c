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
    // NOT_IMPLEMENT     
    // libmaix_err_t libmaix_cv_image_crop(libmaix_image_t *src, int x, int y, int w, int h, libmaix_image_t *dst);
    // libmaix_err_t libmaix_cv_image_resize(libmaix_image_t *src, int w, int h, libmaix_image_t *dst);
    // libmaix_err_t libmaix_cv_image_rotate(libmaix_image_t *src, int rotate);
    // libmaix_err_t libmaix_cv_image_convert(libmaix_image_t *src, libmaix_image_t *dst);

    libmaix_image_module_init();
    struct libmaix_disp *disp = libmaix_disp_create(0);
    if (disp)
    {
        libmaix_image_t *rgb888 = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        if (rgb888)
        {
            printf("w %d h %d p %d \r\n", rgb888->width, rgb888->height, rgb888->mode);

            libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 0, 0, 360, MaixColor(255, 0, 0), 2);
            libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 45, 0, 360, MaixColor(0, 255, 0), 2);
            libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, -45, 0, 360, MaixColor(0, 0, 255), 2);
            libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 90, 0, 360, MaixColor(255, 255, 255), 2);

            libmaix_cv_image_draw_circle(rgb888, 200, 200, 10, MaixColor(255, 0, 0), 1);
            libmaix_cv_image_draw_circle(rgb888, 150, 200, 20, MaixColor(0, 255, 0), 5);
            libmaix_cv_image_draw_circle(rgb888, 200, 150, 30, MaixColor(0, 0, 255), 10);

            disp->draw_image(disp, rgb888);

            libmaix_image_destroy(&rgb888);
        }
        libmaix_disp_destroy(&disp);
    }
    libmaix_image_module_deinit();
    return 0;
}
