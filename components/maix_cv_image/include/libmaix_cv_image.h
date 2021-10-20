
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

libmaix_err_t libmaix_cv_image_draw(libmaix_image_t *src, libmaix_image_t *dst);

libmaix_err_t libmaix_cv_image_draw_circle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness);

libmaix_err_t libmaix_cv_image_draw_ellipse(libmaix_image_t *src, int x, int y, int w, int h, double angle, double startAngle, double endAngle, libmaix_image_color_t color, int thickness);

#ifdef __cplusplus
}
#endif

#endif
