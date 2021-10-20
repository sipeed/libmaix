
#ifndef __LIBMAIX_CV_IMAGE_H__
#define __LIBMAIX_CV_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libmaix_err.h"
#include "libmaix_image.h"

enum CircleLineTypes {
    FILLED  = -1,
    LINE_4  = 4, //!< 4-connected line
    LINE_8  = 8, //!< 8-connected line
    LINE_AA = 16 //!< antialiased line
};

libmaix_err_t libmaix_cv_image_draw_image(libmaix_image_t *src, int x, int y, libmaix_image_t *dst, libmaix_image_color_t color);
libmaix_err_t libmaix_cv_image_draw_image_open(libmaix_image_t *src, int x, int y, const char *path, libmaix_image_color_t color);

libmaix_err_t libmaix_cv_image_draw_ellipse(libmaix_image_t *src, int x, int y, int w, int h, double angle, double startAngle, double endAngle, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_circle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_rectangle(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_line(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness);

libmaix_err_t libmaix_cv_image_load_freetype(const char *path);
libmaix_err_t libmaix_cv_image_draw_string(libmaix_image_t *src, int x, int y, const char *str, libmaix_image_color_t color, double scale, int thickness);

libmaix_err_t libmaix_cv_image_crop(libmaix_image_t *src, int x, int y, int w, int h, libmaix_image_t *dst);
libmaix_err_t libmaix_cv_image_resize(libmaix_image_t *src, int w, int h, libmaix_image_t *dst);
libmaix_err_t libmaix_cv_image_rotate(libmaix_image_t *src, int rotate);
libmaix_err_t libmaix_cv_image_convert(libmaix_image_t *src, libmaix_image_t *dst);

#ifdef __cplusplus
}
#endif

#endif
