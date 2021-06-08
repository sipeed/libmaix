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
#include "yuv2rgb.h"
#include "string.h"


#define TEST_RESIZE_IMAGE 0
#define SOFT_YUV2RGB      0

#if SOFT_YUV2RGB
    #include "yuv2rgb.h"
    #include "string.h"
#else
    #include "libmaix_nn.h"
#endif


void camera_test(struct libmaix_disp* disp)
{
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    libmaix_err_t err0 = LIBMAIX_ERR_NONE;
    struct libmaix_cam* cam     = NULL;
    struct libmaix_cam* cam1     = NULL;
    libmaix_image_t* img        = NULL;
    struct timeval start, end;
    int64_t interval_s;
    uint32_t res_w = 240, res_h = 240;

#if TEST_RESIZE_IMAGE
    libmaix_image_t* resize_img = NULL;
#endif

#define CALC_TIME_START() do{gettimeofday( &start, NULL );}while(0)
#define CALC_TIME_END(name)   do{gettimeofday( &end, NULL ); \
                            interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll; \
                            printf("%s use time: %lld us\n", name, interval_s + end.tv_usec - start.tv_usec);\
        }while(0)
#if !SOFT_YUV2RGB
    printf("--nn module init\n");
    libmaix_nn_module_init();
#endif

    printf("--image module init\n");
    libmaix_image_module_init();

    printf("--create image\n");
    img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_YUV420SP_NV21, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!img)
    {
        printf("create yuv image fail\n");
        // goto end;
    }
    libmaix_image_t* rgb_img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!rgb_img)
    {
        printf("create rgb image fail\n");
        goto end;
    }

    err0 = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, &rgb_img);
    if(err0 != LIBMAIX_ERR_NONE)
    {
        printf("conver to RGB888 fail:%s\r\n", libmaix_get_err_msg(err0));
        return ;
    }

    libmaix_cam_init();

    printf("--create cam\n");
    cam = libmaix_cam_creat(0, res_w, res_h);
    if(!cam)
    {
        printf("create cam fail\n");
        goto end;
    }
    printf("--create cam1\n");
    cam1 = libmaix_cam_creat(1, 1024, 600);
    if(!cam1)
    {
        printf("create cam1 fail\n");
        goto end;
    }
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    err = cam1->start_capture(cam1);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
#if TEST_RESIZE_IMAGE
    resize_img = libmaix_image_create(224, 224, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!resize_img)
    {
        printf("create image error!\n");
        goto end;
    }
#endif

    while(1)
    {
        // printf("--cam capture\n");
        CALC_TIME_START();
        img->mode = LIBMAIX_IMAGE_MODE_YUV420SP_NV21;
        err = cam->capture(cam, (unsigned char*)img->data);
        if(err != LIBMAIX_ERR_NONE)
        {
            // not ready， sleep to release CPU
            if(err == LIBMAIX_ERR_NOT_READY)
            {
                usleep(20 * 1000);
                continue;
            }
            else
            {
                printf("capture fail: %s\n", libmaix_get_err_msg(err));
                break;
            }
        }
        CALC_TIME_END("capture");
        CALC_TIME_START();
        // printf("--conver YUV to RGB\n");
#if SOFT_YUV2RGB
        yuv420sp_to_yuv420p((unsigned char*)img->data , (unsigned char*) rgb_img->data, img->width, img->height, true);
        memcpy(img->data, rgb_img->data, img->width * img->height * 3 / 2);
        yuv420p_to_rgb24((unsigned char*)img->data, (unsigned char*) rgb_img->data, img->width, img->height);
#else
        err0 = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, &rgb_img);
        if(err0 != LIBMAIX_ERR_NONE)
        {
            printf("conver to RGB888 fail:%s\r\n", libmaix_get_err_msg(err0));
            continue;
        }
#endif
        CALC_TIME_END("convert to RGB888");
        CALC_TIME_START();
        // printf("--convert test end\n");
#if TEST_RESIZE_IMAGE
        err0 = rgb_img->resize(rgb_img, resize_img->width, resize_img->height, &resize_img);
        if(err0 != LIBMAIX_ERR_NONE)
        {
            printf("resize image error: %s\r\n", libmaix_get_err_msg(err0));
            continue;
        }
        CALC_TIME_END("resize image");
        CALC_TIME_START();
        disp->draw(disp, resize_img->data, (disp->width - resize_img->width) / 2,(disp->height - resize_img->height) / 2, resize_img->width, resize_img->height, 1);
#else
        disp->draw(disp, (unsigned char*)rgb_img->data, (disp->width - rgb_img->width) / 2,(disp->height - rgb_img->height) / 2, rgb_img->width, rgb_img->height, 1);
#endif
        // disp->flush(disp); // disp->draw last arg=1, means will call flush in draw functioin
        CALC_TIME_END("display");
    }
end:
#if TEST_RESIZE_IMAGE
    if(resize_img)
    {
        printf("--image destory\n");
        libmaix_image_destroy(&resize_img);
    }
#endif
    if(cam)
    {
        printf("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    if(cam1)
    {
        printf("--cam1 destory\n");
        libmaix_cam_destroy(&cam1);
    }
    if(rgb_img)
    {
        printf("--image destory\n");
        libmaix_image_destroy(&rgb_img);
    }
    if(img)
    {
        printf("--image destory\n");
        libmaix_image_destroy(&img);
    }

    libmaix_cam_exit();

    printf("--image module deinit\n");
    libmaix_image_module_deinit();
#if !SOFT_YUV2RGB
    libmaix_nn_module_deinit();
#endif
}

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_creat();
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

