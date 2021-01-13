#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"

#include "libmaix_cam.h"
#include "libmaix_disp.h"
#include "libmaix_image.h"
#include "main.h"
#include <sys/time.h>
#include <unistd.h>


void camera_test(struct libmaix_disp_t* disp)
{
    int ret = 0;
    struct libmaix_cam_t* cam = NULL;
    libmaix_image_t* img;
    struct timeval start, end;
    int64_t interval_s;
    uint32_t res_w = 240, res_h = 240;

#define CALC_TIME_START() do{gettimeofday( &start, NULL );}while(0)
#define CALC_TIME_END(name)   do{gettimeofday( &end, NULL ); \
                            interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll; \
                            printf("%s use time: %lld us\n", name, interval_s + end.tv_usec - start.tv_usec);\
        }while(0)

    printf("--image module init\n");
    libmaix_image_module_init();

    printf("--create image\n");
    img = libmaix_image_creat(res_w, res_h, LIBMAIX_IMAGE_MODE_YUV420SP_NV21, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!img)
    {
        printf("create yuv image fail\n");
        goto end;
    }
    printf("--create cam\n");
    cam = libmaix_cam_creat(res_w, res_h);
    if(!cam)
    {
        printf("create cam fail\n");
        goto end;
    }
    printf("--cam start capture\n");
    ret = cam->strat_capture(cam);
    if(ret != 0)
    {
        printf("start capture fail!!!!\n");
        goto end;
    }

    while(1)
    {
        // printf("--cam capture\n");
        CALC_TIME_START();
        ret = cam->capture(cam, (unsigned char*)img->data);
        if(ret != 0)
        {
            // not ready， sleep to release CPU
            if(ret == 1)
            {
                usleep(20 * 1000);
                continue;
            }
            else
            {
                printf("capture fail, error code: %d\n", ret);
                break;
            }
        }
        CALC_TIME_END("capture");
        CALC_TIME_START();
        // printf("--conver YUV to RGB\n");
        libmaix_image_err_t err0 = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, NULL);
        if(err0 != LIBMAIX_IMAGE_ERR_NONE)
        {
            printf("conver to RGB888 fail:%s\r\n", libmaix_image_get_err_msg(err0));
            continue;
        }
        CALC_TIME_END("convert to RGB888");
        CALC_TIME_START();
        // printf("--convert test end\n");
        disp->draw(disp, img->data, (disp->width - img->width) / 2,(disp->height - img->height) / 2, img->width, img->height, 1);
        // disp->flush(disp); // disp->draw last arg=1, means will call flush in draw functioin
        CALC_TIME_END("display");
    }
end:
    if(cam)
    {
        printf("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    if(img)
    {
        printf("--image destory\n");
        libmaix_image_destroy(&img);
    }
    printf("--image module deinit\n");
    libmaix_image_module_deinit();
}

int main(int argc, char* argv[])
{
    struct libmaix_disp_t* disp = libmaix_disp_creat();
    if(disp == NULL) {
        printf("creat disp object fail\n");
        return -1;
    }

    disp->swap_rb = 1;

    printf("draw test\n");
    camera_test(disp);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}

