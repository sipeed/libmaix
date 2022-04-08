#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"

#include "libmaix_cam.h"
#include "libmaix_disp.h"
#include "libmaix_image.h"
#include "libmaix_nn.h"
#include "main.h"
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <errno.h>
#include "mdsc.h"
#include <string.h>

#define LOAD_IMAGE 0
#if LOAD_IMAGE
#define SAVE_NETOUT 1
#endif
#define debug_line printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)


#define DISPLAY_TIME 0

#if DISPLAY_TIME
    struct timeval start, end;
    int64_t interval_s;
#define CALC_TIME_START()           \
    do                              \
    {                               \
        gettimeofday(&start, NULL); \
    } while (0)
#define CALC_TIME_END(name)                                                               \
    do                                                                                    \
    {                                                                                     \
        gettimeofday(&end, NULL);                                                         \
        interval_s = (int64_t)(end.tv_sec - start.tv_sec) * 1000000ll;                    \
        printf("%s use time: %lld us\n", name, interval_s + end.tv_usec - start.tv_usec); \
    } while (0)
#else
#define CALC_TIME_START()
#define CALC_TIME_END(name)
#endif



static volatile bool program_exit = false;

int loadFromBin(const char *binPath, int size, signed char *buffer)
{
    FILE *fp = fopen(binPath, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "fopen %s failed\n", binPath);
        return -1;
    }
    int nread = fread(buffer, 1, size, fp);
    if (nread != size)
    {
        fprintf(stderr, "fread bin failed %d\n", nread);
        return -1;
    }
    fclose(fp);

    return 0;
}

int save_bin(const char *path, int size, uint8_t *buffer)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "fopen %s failed\n", path);
        return -1;
    }
    int nwrite = fwrite(buffer, 1, size, fp);
    if (nwrite != size)
    {
        fprintf(stderr, "fwrite bin failed %d\n", nwrite);
        return -1;
    }
    fclose(fp);

    return 0;
}

static void softmax(float *data, int n )
{
    int stride = 1;
    int i;
    // int diff;
    // float e;
    float sum = 0;
    float largest_i = data[0];

    for (i = 0; i < n; ++i)
    {
        if (data[i * stride] > largest_i)
            largest_i = data[i * stride];
    }
    for (i = 0; i < n; ++i)
    {
        float value = expf(data[i * stride] - largest_i);
        sum += value;
        data[i * stride] = value;
    }
    for (i = 0; i < n; ++i)
	{
        data[i * stride] /= sum;
	}
}


void nn_test(struct libmaix_disp *disp)
{

    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_nn_module_init();
    libmaix_camera_module_init();

    char *mdsc_path = "/root/mdsc/r329_shufflenet.mdsc";
    uint32_t res_w = 224;
    uint32_t res_h = 224;
    libmaix_nn_t *nn = NULL;
    float* result = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    printf("--cam create\n");
    libmaix_image_t *img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_cam_t *cam = libmaix_cam_create(0, res_w, res_h, 1, 1);
#ifdef CONFIG_ARCH_V831
    libmaix_cam_t *cam2 = libmaix_cam_create(1, disp->width, disp->height, 0, 0);
#endif
    if (!cam)
    {
        printf("create cam fail\n");
    }
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
#ifdef CONFIG_ARCH_V831
    err = cam2->start_capture(cam2);
#endif
    if (err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //input
    libmaix_nn_layer_t input = {
        .w = 224,
        .h = 224,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    //output
    libmaix_nn_layer_t out_fmap = {
        .w = 1,
        .h = 1,
        .c = 1000,
        .layout = LIBMAIX_IMAGE_LAYOUT_CHW,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL
    };
    //input buffer
    int8_t *quantize_buffer = (int8_t *)malloc(input.w * input.h * input.c);
    if (!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    input.buff_quantization = quantize_buffer;
    // output buffer
    float *output_buffer = (float *)malloc(out_fmap.c * out_fmap.w * out_fmap.h * sizeof(float));
    if (!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    out_fmap.data = output_buffer;

    nn = load_mdsc(mdsc_path);
    printf("-- start loop\n");
    while (!program_exit)
    {
#if LOAD_IMAGE
        printf("-- load input bin file\n");
        loadFromBin("/root/test_input/input_256x448.bin", res_w * res_h * 3, rgb_img->data);
#else
        err = cam->capture_image(cam, &img);
#ifdef CONFIG_ARCH_V831
        err = cam2->capture_image(cam2, &show);
#endif

        if (err != LIBMAIX_ERR_NONE)
        {
            // not ready， sleep to release CPU
            if (err == LIBMAIX_ERR_NOT_READY)
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
#endif

        // forward process
        input.data = (uint8_t *)img->data;
        CALC_TIME_START();
        err = nn->forward(nn, &input, &out_fmap);
        CALC_TIME_END("forward");
        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        float max_p = 0;
        int max_idx = 0;
        CALC_TIME_START();
        softmax(out_fmap.data, 1000);
        result = (float*)out_fmap.data;
        for (int i = 0; i < 1000; ++i)
        {
            if (result[i] > max_p)
            {
                max_p = result[i];
                max_idx = i;
            }
        }
        printf("%f::%s \n", max_p, labels[max_idx]);
        printf("____________\n")
        CALC_TIME_END("decode");
        err = libmaix_cv_image_resize(img, disp->width, disp->height, &show);
        disp->draw_image(disp, show);
#if LOAD_IMAGE
        break;
#endif
    }
end:
    if (output_buffer)
    {
        free(output_buffer);
    }
    if (nn)
    {
        libmaix_nn_destroy(&nn);
    }

    if (cam)
    {
        printf("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    printf("--image module deinit\n");
    libmaix_nn_module_deinit();
    libmaix_image_module_deinit();
}

static void handle_signal(int signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
    {
        program_exit = true;
    }
}

int main(int argc, char *argv[])
{
    struct libmaix_disp *disp = libmaix_disp_create(0);
    if (disp == NULL)
    {
        printf("creat disp object fail\n");
        return -1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("program start\n");
    nn_test(disp);
    printf("program end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}
