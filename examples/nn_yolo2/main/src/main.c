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
#include "libmaix_nn_decoder_yolo2.h"
#include "main.h"
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

typedef struct
{
    struct libmaix_disp* disp;
    libmaix_image_t*       img;
}callback_param_t;

int loadFromBin(const char* binPath, int size, signed char* buffer)
{
	FILE* fp = fopen(binPath, "rb");
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

void on_draw_box(uint32_t id, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t class_id, float prob, char* label, void* arg)
{
    callback_param_t* data = (callback_param_t*)arg;
    struct libmaix_disp* disp = data ? data->disp : NULL;
    libmaix_image_t* img = data ? data->img : NULL;
    char* default_label = "unknown";
    // libmaix_err_t err;
    static uint32_t last_id = 0xffffffff;
    if(id != last_id)
    {
        printf("----image:%d----\n", id);
    }
    printf("x:%d, y:%d, w:%d, h:%d, prob:%f, id:%d", x, y, w, h, prob, class_id);
    if(label)
    {
        printf(", label:%s\n", label);
    }
    else
    {
        label = default_label;
        printf("\n");
    }
    libmaix_image_color_t color = {
        .rgb888.r = 255,
        .rgb888.g = 255,
        .rgb888.b = 255
    };
    libmaix_image_color_t color2 = {
        .rgb888.r = 0,
        .rgb888.g = 255,
        .rgb888.b = 0
    };
    if(disp && img)
    {
        img->draw_rectangle(img, x, y, w, h, color, false, 4);
        img->draw_string(img, label, x+4, y+4, 16, color2, NULL);
    }
    last_id = id;
}

void nn_test(struct libmaix_disp* disp)
{
#if TEST_IMAGE
    int ret = 0;
#endif
    int count = 0;
    struct libmaix_cam* cam = NULL;
    libmaix_image_t* img;
    callback_param_t callback_arg;

    libmaix_nn_t* nn = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    uint32_t res_w = 224, res_h = 224;
    char* labels[] = {"aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"};
    int class_num = 20;
    float anchors[10] = {0.375, 0.5, 1.75, 3.1875, 0.9375, 1.34375, 4.625, 4.46875, 3.125, 2.0625};
    uint8_t anchor_len = sizeof(anchors) / sizeof(float) / 2;

    libmaix_nn_decoder_yolo2_config_t yolo2_config = {
        .classes_num     = class_num,
        .threshold       = 0.5,
        .nms_value       = 0.3,
        .anchors_num     = 5,
        .anchors         = anchors,
        .net_in_width    = 224,
        .net_in_height   = 224,
        .net_out_width   = 7,
        .net_out_height  = 7,
        .input_width     = res_w,
        .input_height    = res_w
    };
    libmaix_nn_decoder_t* yolo2_decoder = NULL;

#define TEST_IMAGE   0
#define DISPLAY_TIME 1

#if DISPLAY_TIME
    struct timeval start, end;
    int64_t interval_s;
    #define CALC_TIME_START() do{gettimeofday( &start, NULL );}while(0)
    #define CALC_TIME_END(name)   do{gettimeofday( &end, NULL ); \
                                interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll; \
                                printf("%s use time: %lld us\n", name, interval_s + end.tv_usec - start.tv_usec);\
            }while(0)
#else
    #define CALC_TIME_START() 
    #define CALC_TIME_END(name)
#endif

    printf("--nn module init\n");
    libmaix_nn_module_init();
    printf("--image module init\n");
    libmaix_image_module_init();

    printf("--create image\n");
    img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    // use LIBMAIX_IMAGE_MODE_RGB888 for read RGB88 image from FS test
    // for camera LIBMAIX_IMAGE_MODE_YUV420SP_NV21 is enough
    if(!img)
    {
        printf("create yuv image fail\n");
        goto end;
    }
    libmaix_image_t* rgb_img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!rgb_img)
    {
        printf("create rgb image fail\n");
        goto end;
    }
    // camera init
#if !TEST_IMAGE
    printf("--create cam\n");
    cam = libmaix_cam_creat(res_w, res_h);
    if(!cam)
    {
        printf("create cam fail\n");
        goto end;
    }
    printf("--cam start capture\n");
    err = cam->strat_capture(cam);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
#endif
    printf("--resnet init\n");
    libmaix_nn_model_path_t model_path = {
        .awnn.param_path = "/root/models/yolo2_20class_awnn.param",
        .awnn.bin_path = "/root/models/yolo2_20class_awnn.bin",
    };
    libmaix_nn_layer_t input = {
        .w = yolo2_config.net_in_width,
        .h = yolo2_config.net_in_height,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_UINT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    libmaix_nn_layer_t out_fmap = {
        .w = yolo2_config.net_out_width,
        .h = yolo2_config.net_out_height,
        .c = (class_num + 5) * anchor_len,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL
    };
    char* inputs_names[] = {"input0"};
    char* outputs_names[] = {"output0"};
    libmaix_nn_opt_param_t opt_param = {
        .awnn.input_names             = inputs_names,
        .awnn.output_names            = outputs_names,
        .awnn.first_layer_conv_no_pad = 0, // 0/1
        .awnn.input_num               = 1,              // len(input_names)
        .awnn.output_num              = 1,              // len(output_names)
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.00784313725490196, 0.00784313725490196, 0.00784313725490196},
    };
    float* output_buffer = (float*)malloc(out_fmap.w * out_fmap.h * out_fmap.c * sizeof(float));
    if(!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    uint8_t* quantize_buffer = (uint8_t*)malloc(input.w * input.h * input.c);
    if(!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    out_fmap.data = output_buffer;
    input.buff_quantization = quantize_buffer;
#if TEST_IMAGE
    ret = loadFromBin("/root/test_input/input_data.bin", input.w * input.h * input.c, img->data);
    if(ret != 0)
    {
        printf("read file fail!\n");
        goto end;
    }
    img->mode = LIBMAIX_IMAGE_MODE_RGB888;
#endif
    // nn model init
    printf("-- nn create\n");
    nn = libmaix_nn_creat();
    if(!nn)
    {
        printf("libmaix_nn object create fail\n");
        goto end;
    }
    printf("-- nn object init\n");
    err = nn->init(nn);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    printf("-- nn object load model\n");
    err = nn->load(nn, &model_path, &opt_param);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    // decoder init
    printf("-- yolo2 decoder create\n");
    yolo2_decoder = libmaix_nn_decoder_yolo2_creat();
    if(!yolo2_decoder)
    {
        printf("no mem\n");
        goto end;
    }
    printf("-- yolo2 decoder init\n");
    err = yolo2_decoder->init(yolo2_decoder, (void*)&yolo2_config);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("decoder init error:%d\n", err);
        goto end;
    }
    libmaix_nn_decoder_yolo2_result_t yolo2_result;

    printf("-- start loop\n");
    while(1)
    {
#if !TEST_IMAGE
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
        libmaix_err_t err0 = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, &rgb_img);
        if(err0 != LIBMAIX_ERR_NONE)
        {
            printf("conver to RGB888 fail:%s\r\n", libmaix_get_err_msg(err0));
            continue;
        }
        CALC_TIME_END("convert to RGB888");
#endif
        CALC_TIME_START();
        // printf("--maix nn forward\n");
        input.data = rgb_img->data;
        err = nn->forward(nn, &input, &out_fmap);
        if(err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            break;
        }
        CALC_TIME_END("maix nn forward");
        CALC_TIME_START();
        err = yolo2_decoder->decode(yolo2_decoder, &out_fmap, (void*)&yolo2_result);
        if(err != LIBMAIX_ERR_NONE)
        {
            printf("yolo2 decode fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        CALC_TIME_END("maix nn yolo2 decode");
        callback_arg.disp = disp;
        callback_arg.img = rgb_img;
        if(yolo2_result.boxes_num > 0)
        {
            libmaix_nn_decoder_yolo2_draw_result(yolo2_decoder, &yolo2_result, count++, labels, on_draw_box, (void*)&callback_arg);
        }
        disp->draw(disp, rgb_img->data, (disp->width - rgb_img->width) / 2,(disp->height - rgb_img->height) / 2, rgb_img->width, rgb_img->height, 1);
        // disp->flush(disp); // disp->draw last arg=1, means will call flush in draw functioin
        CALC_TIME_START();
        // printf("--convert test end\n");
        CALC_TIME_END("display");
#if TEST_IMAGE
        break;
#endif
    }
end:
    if(yolo2_decoder)
    {
        libmaix_nn_decoder_yolo2_destroy(&yolo2_decoder);
    }
    if(output_buffer)
    {
        free(output_buffer);
    }
    if(nn)
    {
        libmaix_nn_destroy(&nn);
    }
    if(cam)
    {
        printf("--cam destory\n");
        libmaix_cam_destroy(&cam);
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
    printf("--image module deinit\n");
    libmaix_nn_module_deinit();
    libmaix_image_module_deinit();
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
    nn_test(disp);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}

