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
  libmaix_image_module_init();
  struct libmaix_disp *disp = libmaix_disp_create(0);
  if (disp)
  {
    libmaix_image_t *rgb888 = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if (rgb888)
    {
      printf("w %d h %d p %d \r\n", rgb888->width, rgb888->height, rgb888->mode);

      // libmaix_cv_image_test(rgb888, rgb888);

      // libmaix_cv_image_draw_image_open(rgb888, 20, 20, "/home/res/logo.png");

      libmaix_cv_image_draw_rectangle(rgb888, 0, 0, 240, 240, MaixColor(255, 255, 255), -1);

      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 0, 0, 360, MaixColor(255, 0, 0), 2);

      //   libmaix_image_t *tmp = libmaix_image_create(this->img->width, this->img->height, this->img->mode, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
      //   libmaix_cv_image_crop(this->img, thr[0], thr[1], thr[2], thr[3], &tmp);
      //   libmaix_image_destroy(&this->img), this->img = tmp;
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 45, 0, 360, MaixColor(0, 255, 0), 2);
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, -45, 0, 360, MaixColor(0, 0, 255), 2);
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 90, 0, 360, MaixColor(55, 55, 55), 2);

      // libmaix_cv_image_draw_circle(rgb888, 200, 200, 10, MaixColor(255, 0, 0), 1);
      // libmaix_cv_image_draw_circle(rgb888, 150, 200, 20, MaixColor(0, 255, 0), 5);
      // libmaix_cv_image_draw_circle(rgb888, 200, 150, 30, MaixColor(0, 0, 255), 10);

      libmaix_cv_image_draw_rectangle(rgb888, 10, 10, 130, 120, MaixColor(255, 0, 0), 2);
      libmaix_cv_image_draw_line(rgb888, 10, 10, 130, 120, MaixColor(255, 0, 0), 2);
      libmaix_cv_image_draw_string(rgb888, 0, 120, "test123[]-=", 1.0, MaixColor(255, 0, 255), 2);
      // libmaix_cv_image_load_freetype("./txwzs.ttf");
      // libmaix_cv_image_draw_string(rgb888, 0, 0, u8"123你好鸭asdにほんご", 0.8, MaixColor(55, 55, 55), 1);

      // libmaix_cv_image_draw_string(rgb888, 0, 0, u8"123你好鸭asdにほんご", MaixColor(55, 55, 55), 0.8, 1);

      // rgb888->draw_string(0, 0, "123");

      disp->draw_image(disp, rgb888);

      libmaix_image_destroy(&rgb888);
    }
    libmaix_disp_destroy(&disp);
  }
  libmaix_image_module_deinit();
  return 0;
}
