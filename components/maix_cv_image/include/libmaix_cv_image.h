
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
libmaix_err_t libmaix_cv_image_open_file(libmaix_image_t **src, const char *path);

libmaix_err_t libmaix_cv_image_draw_image(libmaix_image_t *src, int x, int y, libmaix_image_t *dst, double opacity);
libmaix_err_t libmaix_cv_image_draw_image_open(libmaix_image_t *src, int x, int y, const char *path, double opacity);
libmaix_err_t libmaix_cv_image_draw_image_save(libmaix_image_t *src, const char *path);

libmaix_err_t libmaix_cv_image_draw_ellipse(libmaix_image_t *src, int x, int y, int w, int h, double angle, double startAngle, double endAngle, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_circle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_rectangle(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness);
libmaix_err_t libmaix_cv_image_draw_line(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness);

libmaix_err_t libmaix_cv_image_load_freetype(const char *path);
void libmaix_cv_image_get_string_size(int *width, int *height, const char *str, double scale, int thickness);
libmaix_err_t libmaix_cv_image_draw_string(libmaix_image_t *src, int x, int y, const char *str, double scale, libmaix_image_color_t color, int thickness);

libmaix_err_t libmaix_cv_image_crop(libmaix_image_t *src, int x, int y, int w, int h, libmaix_image_t **dst);
libmaix_err_t libmaix_cv_image_resize(libmaix_image_t *src, int w, int h, libmaix_image_t **dst);
libmaix_err_t libmaix_cv_image_rotate(libmaix_image_t *src, double rotate, int adjust, libmaix_image_t **dst);
libmaix_err_t libmaix_cv_image_convert(libmaix_image_t *src, libmaix_image_mode_t type, libmaix_image_t **dst);

libmaix_image_color_t libmaix_cv_image_get_pixel(libmaix_image_t *src, int x, int y);
libmaix_err_t libmaix_cv_image_set_pixel(libmaix_image_t *src, int x, int y, libmaix_image_color_t color);
#ifdef __cplusplus
}
#endif

#endif
