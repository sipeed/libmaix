#ifndef __LIBMAIX_DISP_H
#define __LIBMAIX_DISP_H

#include <stdio.h>
#include <stdlib.h>
#include "libmaix_err.h"

typedef struct libmaix_disp
{
    int width;      /* FB的宽度 */
    int height;     /* FB的高度 */
    int swap_rb;    /* FB是否需要交换RB */

    /**
     * @brief 绘制图像
     * 
     * @param [in] disp: disp对象
     * @param [in] buf: 图像内存地址
     * @param [in] x: 图像左上角的x偏移
     * @param [in] y: 图像左上角的x偏移
     * @param [in] w: 图像的宽度
     * @param [in] h: 图像的高度
     * @param [in] flush: 自动刷新
     * 
     * @return 0
      */
    libmaix_err_t (*draw)(struct libmaix_disp *disp, unsigned char *buf, int x, int y, int w, int h, int flush);

    /**
     * @brief 更新绘制的数据到屏幕
     * 
     * @param [in] disp: disp对象
     */ 
    libmaix_err_t (*flush)(struct libmaix_disp *disp);

    void *reserved;
}libmaix_disp_t;

/**
 * @brief 创建disp对象
 * 
 * @return 创建的对象；NULL:出错
*/
struct libmaix_disp * libmaix_disp_creat(void);

/**
 * @brief 销毁disp对象
*/
void libmaix_disp_destroy(struct libmaix_disp** disp);

#endif /* __LIBMAIX_DISP_H */
