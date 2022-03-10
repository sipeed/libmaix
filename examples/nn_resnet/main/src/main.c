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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <errno.h>

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
    struct libmaix_cam *cam = NULL;
    libmaix_image_t* img;

    uint32_t res_w = 224, res_h = 224;
    libmaix_nn_t *nn = NULL;
    float* result = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;

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

    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_camera_module_init();
    libmaix_nn_module_init();

    printf("--create image\n");
    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!show)
    {
        printf("create RGB image fail\n");
        goto end;
    }

    printf("--create cam\n");
    cam = libmaix_cam_create(0, res_w, res_h, 1, 0);
    if (!cam)
    {
        printf("create cam fail\n");
        goto end;
    }
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
    if (err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail:%s\n", libmaix_get_err_msg(err));
        goto end;
    }

    printf("--resnet init\n");
    libmaix_nn_model_path_t model_path = {
        // .awnn.param_path = "./resnet18_1000_awnn.param",
        // .awnn.bin_path = "./resnet18_1000_awnn.bin"
        .aipu.model_path = "/root/models/aipu_resnet50.bin"
    };
    libmaix_nn_layer_t input = {
        .w = 224,
        .h = 224,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_UINT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL};
    libmaix_nn_layer_t out_fmap = {
        .w = 1,
        .h = 1,
        .c = 1000,
        .layout = LIBMAIX_IMAGE_LAYOUT_CHW,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL};

    char *inputs_names[] = {"input0"};
    char *outputs_names[] = {"output0"};
    libmaix_nn_opt_param_t opt_param = {
        .aipu.input_names = inputs_names,
        .aipu.output_names = outputs_names,
        .aipu.input_num = 1,  // len(input_names)
        .aipu.output_num = 1, // len(output_names)
        .aipu.mean = {127.5, 127.5, 127.5},
        .aipu.norm = {0.00784313725490196, 0.00784313725490196, 0.00784313725490196},
        .aipu.scale  = {7.5395403},
    };


    float *output_buffer = (float *)malloc(1000 * sizeof(float));
    if (!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    uint8_t *quantize_buffer = (uint8_t *)malloc(input.w * input.h * input.c);  // origin branch is uint8
    if (!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    input.buff_quantization = quantize_buffer;
    out_fmap.data = output_buffer;

    printf("-- nn create\n");
    nn = libmaix_nn_create();
    if (!nn)
    {
        printf("libmaix_nn object create fail\n");
        goto end;
    }
    printf("-- nn object init\n");
    err = nn->init(nn);
    if (err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    printf("-- nn object load model\n");
    err = nn->load(nn, &model_path, &opt_param);
    if (err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    while (1)
    {
        // CALC_TIME_START();

        err = cam->capture_image(cam, &img);
        err = libmaix_cv_image_resize(img, 240, 240, &show);
        disp->draw_image(disp, show);
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
                printf("capture fail, error code: %s\n", libmaix_get_err_msg(err));
                break;
            }
        }
        input.data = (uint8_t *)img->data;
        CALC_TIME_START();
        err = nn->forward(nn, &input, &out_fmap);

        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            break;
        }
        float max_p = 0;
        int max_idx = 0;
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
        CALC_TIME_END("maix nn forward");

        CALC_TIME_START();
        // printf("--convert test end\n");
        // libmaix_image_color_t color ={
        //     .rgb888.r = 255,
        //     .rgb888.g = 0,
        //     .rgb888.b = 0
        // };
        // char temp_str[100];
        // snprintf(temp_str, 100, "%f, %s", max_p, labels[max_idx]);
        // rgb_img->draw_string(rgb_img, temp_str, 4, 4, 16, color, NULL);
        // disp->draw(disp, rgb_img->data);
        // CALC_TIME_END("display");
    }

end:
    if (output_buffer)
    {
        free(output_buffer);
    }
    if (nn)
    {
        nn->deinit(nn);
        printf("--nn destory\n");
        libmaix_nn_destroy(&nn);
    }
    if (cam)
    {
        printf("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    // if(img)
    // {
    //     printf("--image destory\n");
    //     libmaix_image_destroy(&img);
    // }
    if (show)
    {
        printf("-- caputer destory\n");
        libmaix_image_destroy(&show);
    }
    printf("--image module deinit\n");
    libmaix_nn_module_deinit();
    libmaix_image_module_deinit();
    libmaix_camera_module_deinit();
}

int main(int argc, char *argv[])
{
    struct libmaix_disp *disp = libmaix_disp_create(0);
    if (disp == NULL)
    {
        printf("creat disp object fail\n");
        return -1;
    }

    printf("draw test\n");
    nn_test(disp);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}