
#ifndef __LIBMAIX_CAM_PRIV_H
#define __LIBMAIX_CAM_PRIV_H

#include "libmaix_cam.h"
#include "libmaix_image.h"

#include "v4l2_capture.hpp"

extern "C" {

struct libmaix_cam_priv_t
{
    V4L2Capture *vcap;
    unsigned char inited;
    unsigned char vi_dev;
    unsigned short vi_x;
    unsigned short vi_y;
    unsigned short vi_w;
    unsigned short vi_h;
    unsigned short vi_m;
    unsigned short vi_f;

    libmaix_image_t* vi_img;

    int (*devInit)(struct libmaix_cam *cam);
    int (*devDeinit)(struct libmaix_cam *cam);
};

int cam_priv_init(struct libmaix_cam *cam);

}

#endif /* __LIBMAIX_CAM_PRIV_H */
