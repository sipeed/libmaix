#ifndef __LIBMAIX_DISP_PRIV_H
#define __LIBMAIX_DISP_PRIV_H

#include "libmaix_disp.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/fb.h>

struct libmaix_disp_priv_t
{
    // char *fbp;
    // int fbfd;
    // int fbiopan;
    // struct fb_var_screeninfo vinfo;
    // struct fb_fix_screeninfo finfo;

    libmaix_image_t* disp_img;

    int (*devInit)(struct libmaix_disp *disp);
    int (*devDeinit)(struct libmaix_disp *disp);
};

int disp_priv_init(struct libmaix_disp *disp);

#ifdef __cplusplus
}
#endif

#endif /* __LIBMAIX_DISP_PRIV_H */
