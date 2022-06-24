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
    int w = 320, h = 240;
    libmaix_image_t *rgb888 = libmaix_image_create(w, h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if (rgb888)
    {
      printf("w %d h %d p %d \r\n", rgb888->width, rgb888->height, rgb888->mode);

      libmaix_cv_image_draw_rectangle(rgb888, 0, 0, w, h, MaixColor(255, 127, 255), -1);

      libmaix_cv_image_draw_image_open(rgb888, 20, 20, "/home/res/logo.png", 1.0);

      libmaix_cv_image_draw_image_open(rgb888, 40, 100, "/home/res/face.png", -1.0);

      libmaix_image_t *tmp = libmaix_image_create(80, 80, rgb888->mode, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);

      libmaix_cv_image_crop(rgb888, 40, 40, 80, 80, &tmp);

      libmaix_cv_image_draw_image(rgb888, 0, 0, tmp, 1.0);

      libmaix_cv_image_draw_image(rgb888, 0, 200, tmp, 1.0);

      libmaix_cv_image_draw_image(rgb888, 200, 0, tmp, 1.0);

      libmaix_cv_image_draw_image(rgb888, 200, 200, tmp, 1.0);

      libmaix_image_destroy(&tmp);

      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 0, 0, 360, MaixColor(255, 0, 0), 2);
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 45, 0, 360, MaixColor(0, 255, 0), 2);
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, -45, 0, 360, MaixColor(0, 0, 255), 2);
      libmaix_cv_image_draw_ellipse(rgb888, 120, 120, 100, 25, 90, 0, 360, MaixColor(55, 55, 55), 2);

      libmaix_cv_image_draw_circle(rgb888, 200, 200, 10, MaixColor(255, 0, 0), 1);
      libmaix_cv_image_draw_circle(rgb888, 150, 200, 20, MaixColor(0, 255, 0), 5);
      libmaix_cv_image_draw_circle(rgb888, 200, 150, 30, MaixColor(0, 0, 255), 10);

      libmaix_cv_image_draw_rectangle(rgb888, 0, 0, 120, 22, MaixColor(255, 0, 0), 1);
      libmaix_cv_image_draw_line(rgb888, 10, 10, 130, 120, MaixColor(255, 0, 0), 2);

      int str_w = 0, str_h = 0;

      libmaix_cv_image_get_string_size(&str_w, &str_h, "[A|B|C|D][]!H/~`_-=", 1.5, 1);

      printf("textSize w %d h %d\r\n", str_w, str_h);

      libmaix_cv_image_draw_string(rgb888, 0, 0, "[A|B|C|D][]!H/~`_-=", 1.5, MaixColor(0, 255, 0), 1);

      // libmaix_cv_image_load_freetype("/home/res/sans.ttf");

      libmaix_cv_image_get_string_size(&str_w, &str_h, "[A|B|佬鼠][]!H/~`_-=", 1.5, 1);

      printf("textSize w %d h %d\r\n", str_w, str_h);

      libmaix_cv_image_draw_string(rgb888, 0, 0, "[A|B|佬鼠][]!H/~`_-=", 1.5, MaixColor(0, 0, 255), 1);

      libmaix_cv_image_draw_string(rgb888, 60, 120, u8"123你好鸭asdにほんご", 2.8, MaixColor(55, 55, 55), 1);

      libmaix_image_t *rs = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
      if (rs) {
          libmaix_cv_image_resize_with_padding(rgb888, disp->width, disp->height, &rs);
          disp->draw_image(disp, rs);
          libmaix_image_destroy(&rs);
      }
      libmaix_image_destroy(&rgb888);
    }
    libmaix_disp_destroy(&disp);
  }
  libmaix_image_module_deinit();

  system("sleep 1");

  return 0;
}
