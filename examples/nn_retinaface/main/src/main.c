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
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>

#include "libmaix_nn_decoder_retinaface.h"
#include "main.h"
#include "time_utils.h"

#define LOAD_IMAGE 0

#if LOAD_IMAGE
    #define SAVE_NETOUT 1
#endif

static volatile bool program_exit = false;


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

int save_bin(const char* path, int size, uint8_t* buffer)
{
    FILE* fp = fopen(path, "wb");
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


void nn_test(struct libmaix_disp* disp)
{
    libmaix_image_t* img = NULL;

    int res_w = 224, res_h = 224;
    int input_w = res_w, input_h = res_h;
    int disp_w = 240, disp_h = 240;
    libmaix_nn_t* nn = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    libmaix_nn_decoder_t* decoder = NULL;
    libmaix_nn_decoder_retinaface_result_t result;


    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_nn_module_init();
    libmaix_camera_module_init();
    printf("--cam create\n");
    libmaix_cam_t* cam = libmaix_cam_create(0, res_w, res_h, 1, 0);
    if(!cam)
    {
        printf("create cam fail\n");
    }
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    printf("--create image\n");
    img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_YUV420SP_NV21, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
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
    libmaix_image_t* img_disp = libmaix_image_create(disp_w, disp_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!img_disp)
    {
        printf("create rgb image fail\n");
        goto end;
    }

    printf("--init\n");
    libmaix_nn_model_path_t model_path = {
        .awnn.param_path = "/root/models/model_int8.param",
        .awnn.bin_path = "/root/models/model_int8.bin",
    };
    libmaix_nn_decoder_retinaface_config_t config = {
        .variance = {0.1, 0.2},
        .steps = {8, 16, 32},
        .min_sizes = {16, 32, 64, 128, 256, 512},
        .nms = 0.4,
        .score_thresh = 0.5,
        .input_w = input_w,
        .input_h = input_h
    };
    libmaix_nn_layer_t input = {
        .w = input_w,
        .h = input_h,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    printf("-- init decoder\n");
    decoder = libmaix_nn_decoder_retinaface_create();
    err = decoder->init(decoder, &config);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    printf("-- channel num: %d\n", config.channel_num);
    libmaix_nn_layer_t out_fmap[3] = {
        {
            .w = 4,
            .h = 1,
            .c = config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW
        },
        {
            .w = 2,
            .h = 1,
            .c = config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW
        },
        {
            .w = 10,
            .h = 1,
            .c = config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW
        }
    };
    char* inputs_names[] = {"input0"};
    char* outputs_names[] = {"output0", "output1", "output2"};
    libmaix_nn_opt_param_t opt_param = {
        .awnn.input_names             = inputs_names,
        .awnn.output_names            = outputs_names,
        // .awnn.input_ids               = NULL,
        // .awnn.output_ids              = NULL,
        .awnn.encrypt                 = false,
        .awnn.input_num               = 1,              // len(input_names)
        .awnn.output_num              = 3,              // len(output_names)
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.0078125, 0.0078125, 0.0078125},
    };
    float* output_buffer = (float*)malloc(out_fmap[0].c * out_fmap[0].w * out_fmap[0].h * sizeof(float));
    if(!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    out_fmap[0].data = output_buffer;
    float* output_buffer2 = (float*)malloc(out_fmap[1].c * out_fmap[1].w * out_fmap[1].h * sizeof(float));
    if(!output_buffer2)
    {
        printf("no memory!!!\n");
        goto end;
    }
    out_fmap[1].data = output_buffer2;
    float* output_buffer3 = (float*)malloc(out_fmap[2].c * out_fmap[2].w * out_fmap[2].h * sizeof(float));
    if(!output_buffer3)
    {
        printf("no memory!!!\n");
        goto end;
    }
    out_fmap[2].data = output_buffer3;
    int8_t* quantize_buffer = (int8_t*)malloc(input.w * input.h * input.c);
    if(!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    input.buff_quantization = quantize_buffer;
    printf("-- nn create\n");
    nn = libmaix_nn_create();
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
    printf("-- start loop\n");
    while(!program_exit)
    {
#if LOAD_IMAGE
        printf("-- load input bin file\n");
        loadFromBin("/root/test_input/input_256x448.bin", res_w * res_h * 3, rgb_img->data);
#else
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
        printf("-- got yuv image, width: %d, height:%d\n", img->width, img->height);
        printf("-- convert YUV to RGB\n");
        err = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, &rgb_img);
        if(err != LIBMAIX_ERR_NONE)
        {
            printf("conver to RGB888 fail:%s\r\n", libmaix_get_err_msg(err));
            continue;
        }
#endif
        printf("-- nn object forward model\n");
        input.data = rgb_img->data;
        // input.data = quantize_buffer;
        // for(int i=0; i < 448 * 448; ++i)
        // {
        //     quantize_buffer[i * 3]     = (int) (((uint8_t*)rgb_img->data)[i * 3])     - 128;
        //     quantize_buffer[i * 3 + 1] = (int) (((uint8_t*)rgb_img->data)[i * 3 + 1]) - 128;
        //     quantize_buffer[i * 3 + 2] = (int) (((uint8_t*)rgb_img->data)[i * 3 + 2]) - 128;
        // }
        CALC_TIME_START();
        err = nn->forward(nn, &input, out_fmap);
        CALC_TIME_END("forward");
        if(err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        printf("-- nn object forward model complete\n");
#if SAVE_NETOUT
        save_bin("loc.bin", out_fmap[0].w * out_fmap[0].h * out_fmap[0].c * sizeof(float), out_fmap[0].data);
        save_bin("conf.bin", out_fmap[1].w * out_fmap[1].h * out_fmap[1].c * sizeof(float), out_fmap[1].data);
        save_bin("landmark.bin", out_fmap[2].w * out_fmap[2].h * out_fmap[2].c * sizeof(float), out_fmap[2].data);
#endif

        printf("-- now decode net out\n");
        CALC_TIME_START();
        decoder->decode(decoder,out_fmap, &result);
        CALC_TIME_END("decode face info");
        printf("valid box num: %d\n", result.num);

        printf("-- decode complete\n");
        libmaix_image_color_t color = {
            .rgb888.r = 255,
            .rgb888.g = 0,
            .rgb888.b = 0
        };
        printf("-- draw\n");
        rgb_img->resize(rgb_img, disp_w, disp_h, &img_disp);
        // memcpy(img_disp->data, rgb_img->data, rgb_img->width * rgb_img->height * 3);
        for(int i=0; i < result.num; ++i)
        {
            if(result.faces[i].score > config.score_thresh)
            {
                img_disp->draw_rectangle(img_disp, result.faces[i].box.x * img_disp->width, result.faces[i].box.y * img_disp->height, result.faces[i].box.w * img_disp->width, result.faces[i].box.h * img_disp->height, color, false, 3);
                for(int j=0; j<5; ++j)
                {
                    img_disp->draw_rectangle(img_disp, result.faces[i].points[j * 2] * img_disp->width, result.faces[i].points[j * 2 + 1] * img_disp->height, 2, 2, color, false, 2);
                }
            }
        }
        disp->draw(disp, img_disp->data);
        printf("-- draw complete\n");
#if LOAD_IMAGE
        break;
#endif
    }
end:
    if(output_buffer)
    {
        free(output_buffer);
    }
    if(output_buffer2)
    {
        free(output_buffer2);
    }
    if(output_buffer3)
    {
        free(output_buffer3);
    }
    if(nn)
    {
        libmaix_nn_destroy(&nn);
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
    if(cam)
        libmaix_cam_destroy(&cam);
    if(decoder)
    {
        decoder->deinit(decoder);
        libmaix_nn_decoder_destroy(&decoder);
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

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_create();
    if(disp == NULL) {
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

