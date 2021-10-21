#ifndef __LIBMAIX_DISP_H
#define __LIBMAIX_DISP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libmaix_err.h"
#include "libmaix_image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libmaix_disp
{
    int width;      /* FB的宽度 */
    int height;     /* FB的高度 */
    int bpp;     /* FB的数据格式 RGB8 RGB565 RGB888 RGB8888 */

    /**
     * @brief 绘制图像到 framebuffer ，根据 width * height * bpp 进行 memcpy
     * 
     * @param [in] disp: disp对象
     * @param [in] buf: 自行根据系统情况准备图像内存地址，看示例代码来使用就行
     * 
     * @return 0
      */
    libmaix_err_t (*draw)(struct libmaix_disp *disp, unsigned char *buf);

    /**
     * @brief 绘制图像到 framebuffer (libmaix_image)
     * 
     * @param [in] disp: disp对象
     * @param [in] img: 传入 libmaix_image 对象其内部会自行转换并显示。
     * 
     * @return 0
      */
    libmaix_err_t (*draw_image)(struct libmaix_disp *disp, struct libmaix_image *img);

    void *reserved;
}libmaix_disp_t;

/**
 * @brief 创建disp对象
 * 
 * @param [in] fbiopan: draw 是否启用 fbiopan
 * @return 创建的对象；NULL:出错
*/
struct libmaix_disp * libmaix_disp_create(uint8_t fbiopan);

/**
 * @brief 销毁disp对象
*/
void libmaix_disp_destroy(struct libmaix_disp** disp);

#ifdef __cplusplus
}
#endif


#endif /* __LIBMAIX_DISP_H */
