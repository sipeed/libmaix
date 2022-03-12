#ifndef __LIBJPEG_H
#define __LIBJPEG_H

#include <stdint.h>

typedef struct _jpeg_img
{
    uint32_t w;
    uint32_t h;
    uint32_t bpp;
    uint8_t *data;
} jpeg_img_t;

uint8_t libjpeg_compress(jpeg_img_t *img, uint8_t quality, uint8_t **jpeg_buf, uint64_t *jpeg_len);

//libjpeg_decompress内部存在动态内存分配，需要调用libjpeg_decompress_free释放
uint8_t libjpeg_decompress(jpeg_img_t *jpeg, uint8_t *jpeg_buf, uint32_t jpeg_len);

void libjpeg_decompress_free(jpeg_img_t *jpeg);

#endif /* __LIBJPEG_H */
