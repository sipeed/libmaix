
#ifndef __LIBMAIX_CAM_PRIV_H
#define __LIBMAIX_CAM_PRIV_H

#include "libmaix_cam.h"
#include "libmaix_image.h"

#include "sample_resize.h"
// #include "ax_pool_type.h"

extern "C" {

struct libmaix_cam_priv_t
{
    unsigned char inited;
    unsigned char vi_dev;
    unsigned short vi_x;
    unsigned short vi_y;
    unsigned short vi_w;
    unsigned short vi_h;
    unsigned short vi_m;
    unsigned short vi_f;

    libmaix_image_t* vi_img;

    ax_crop_resize_nv12* nv12_resize_helper;
    void* ivps_out_data;

    // AX_BLK nv12_resize_input_frame_blkid;
    // AX_VIDEO_FRAME_S nv12_resize_input_frame;

    int (*devInit)(struct libmaix_cam *cam);
    int (*devDeinit)(struct libmaix_cam *cam);
};

int cam_priv_init(struct libmaix_cam *cam);

}

#endif /* __LIBMAIX_CAM_PRIV_H */
