#ifndef __LIBMAIX_CAM_H
#define __LIBMAIX_CAM_H

#include <stdio.h>
#include <stdlib.h>
#include "libmaix_err.h"
#include "libmaix_image.h"

#ifdef __cplusplus
extern "C" {
#endif

void libmaix_camera_module_init();

void libmaix_camera_module_deinit();

typedef struct libmaix_cam
{
    int width;      /* 摄像头采集图像的宽度 */
    int height;     /* 摄像头采集图像的高度 */

    unsigned int fram_size; /* 一帧图片占用内存大小 */

    libmaix_err_t (*start_capture)(struct libmaix_cam *cam);

    /**
     * @brief 获取一帧图像
     * 
     * @param [in] cam: cam对象
     * @param [in] buf: 图片缓存内存地址，
     *            buf至少要有`fram_size`这么大的空间
     * 
     * @return 0:成功，1: 未准备好， 其他: 出错
    */
    libmaix_err_t (*capture)(struct libmaix_cam *cam, unsigned char *buf);

    /**
     * @brief 获取一帧图像
     * 
     * @param [in] cam: cam对象
     * @param [in] img: img对象指针（由内部提供）
     * 
     * @return 0:成功，1: 未准备好， 其他: 出错
    */
    libmaix_err_t (*capture_image)(struct libmaix_cam *cam, struct libmaix_image **img);

    void *reserved;
}libmaix_cam_t;

/**
 * @brief 创建cam对象
 * 
 * @param [in] w: 设置采集图像的宽度
 * @param [in] h: 设置采集图像的高度
 * @param [in] m: 设置摄像头水平翻转
 * @param [in] f: 设置摄像头垂直翻转
 * 
 * @return 创建的对象；NULL:出错
*/
struct libmaix_cam * libmaix_cam_create(int n, int w, int h, int m, int f);

/**
 * @brief 销毁cam对象
*/
void libmaix_cam_destroy(struct libmaix_cam **cam);

// -----------mpp_vo-------------------

typedef struct libmaix_vo
{
    void * (*get_frame)(struct libmaix_vo *vo, int layer);
    libmaix_err_t (*set_frame)(struct libmaix_vo *vo, void *frame, int layer);
    libmaix_err_t (*frame_addr)(struct libmaix_vo *vo, void *frame, unsigned int **viraddr, unsigned int **phyaddr);
    void *reserved;
}libmaix_vo_t;

struct libmaix_vo * libmaix_vo_create(int in_w, int in_h, int out_x, int out_y, int out_w, int out_h);

void libmaix_vo_destroy(struct libmaix_vo **vo);

#ifdef __cplusplus
}
#endif

// ----------- g2d ----------------

void g2d_init();

void g2d_exit();

int g2d_nv21_rotate(unsigned char *buf, int w, int h, int rot);

int g2d_argb_rotate(unsigned int *in_buf, void *out_phy, int w, int h, int rot);

int _g2d_nv21_rotate(void *s_vir_0, void *s_vir_1, void *d_vir_0, void *d_vir_1, int w, int h, int rot);

int _g2d_argb_rotate(void *in_phy, void *out_phy, int w, int h, int rot);

int g2d_nv21_to_rgb(unsigned char *in, unsigned char *out, int w, int h);

// -----------

#endif /* __LIBMAIX_CAM_H */
