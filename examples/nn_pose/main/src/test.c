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
#include "libmaix_debug.h"
#include "libmaix_cv_image.h"
#include "libmaix_nn_decoder_pose.h"

#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include "main.h"
#include "time_utils.h"
#include <time.h>

//load weight from file
int get_bin_size(char * filename)
{
    int size = 0;
    FILE * fp = fopen(filename , "rb");
    if(fp)
    {
        fseek(fp , 0 , SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    LIBMAIX_DEBUG_PRINTF("\nfilename=%s,size=%d \n",filename,siz);
    return size;
}

libmaix_err_t read_bin(char *path, void *buf, int size)
{
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    FILE * fp ;
    if((fp = fopen(path , "rb")) == NULL)
    {
        LIBMAIX_DEBUG_PRINTF("\nCan not open the path: %s \n", path);
        err = LIBMAIX_ERR_NOT_EXEC;
        return err;
    }
    fread(buf , sizeof(char) , size , fp);
    fclose(fp);
    return err;
}


//FPS
#define CALC_FPS(tips)                                                                                         \
    {                                                                                                          \
        static int fcnt = 0;                                                                                   \
        fcnt++;                                                                                                \
        static struct timespec ts1, ts2;                                                                       \
        clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
        if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
        {                                                                                                      \
            printf("%s => H26X FPS:%d     \r\n", tips, fcnt);                                                  \
            ts1 = ts2;                                                                                         \
            fcnt = 0;                                                                                          \
        }                                                                                                      \
    }

static volatile bool program_exit = false;

void nn_test(struct libmaix_disp *disp)
{
    //module init
    LIBMAIX_DEBUG_PRINTF("--image module init\n");
    libmaix_image_module_init();
    LIBMAIX_DEBUG_PRINTF("--nn module init\n");
    libmaix_nn_module_init();
    LIBMAIX_DEBUG_PRINTF("--cam module init\n");
    libmaix_camera_module_init();

    // process states
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    //input config
    int input_h = 192 , input_w = 192;

    // image object create
    libmaix_image_t *img = libmaix_image_create(input_w, vfork, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);

    //camera object create1
    libmaix_cam_t *cam = libmaix_cam_create(0, input_w, input_h, 1, 1);
    libmaix_cam_t *cam2 = libmaix_cam_create(1, disp->width, disp->height, 0, 0);
    if (!cam)
    {
        LIBMAIX_DEBUG_PRINTF("create cam fail\n");
    }
    err = cam->start_capture(cam);
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    err = cam2->start_capture(cam2);
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //model config
    libmaix_nn_model_path_t model = {
        .awnn.bin_path = "model_int8.bin",
        .awnn.param_path = "model_int8.param",
    };

    // char * center_weight = "center_weight.bin";
    // char * range_weight_x = "range_weight_x.bin";
    // char * range_weight_y = "range_weight_y.bin";


    //model opt params
    char * input_names [] = {"input0"};
    char * output_names []  = {"output0", "output1", "output2","output3"};

    libmaix_nn_opt_param_t param = {
        .awnn.input_names = input_names,
        .awnn.output_names = output_names,
        .awnn.encrypt = false,
        .awnn.input_num = 1,
        .awnn.output_num = 4,
        .awnn.mean = {127.5, 127.5, 127.5},
        .awnn.norm = {0.0078125, 0.0078125, 0.0078125},
    };

    //decoder config
    libmaix_nn_decoder_pose_config_t decoder_config = {
        .hm_th = 0.1,
        .image_size = 192,
        .num_joints = 17,
        .center_weight = "center_weight.bin",
        .range_weight_x = "range_weight_x.bin",
        .range_weight_y = "range_weight_y.bin",
    };

    //decoder object create and init
    libmaix_nn_decoder_t * pose_decoder = libmaix_nn_decoder_pose_create();
    //check decoder
    err = pose_decoder->init(pose_decoder , &decoder_config);
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //decoder result object
    libmaix_nn_decoder_pose_result_t pose_result ;

    // nn layer input
    libmaix_nn_layer_t input = {
        .w = input_w,
        .h = input_h,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    if(input.need_quantization)
    {
        int8_t * input_buffer = (int8_t * ) malloc (input.c * input_h  * input_w);
        if(! input_buffer)
        {
            LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
            goto end;
        }
        input.buff_quantization = input_buffer;
    }

    // nn layer output
    libmaix_nn_layer_t outputs [4] = {
        // heatmap
        {
            .w = 48,
            .h = 48,
            .c = decoder_config.num_joints,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL ,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        },
        //center
        {
            .w = 48,
            .h = 48,
            .c = 1,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        },
        //reg
        {
            .w = 48,
            .h = 48,
            .c =2 * decoder_config.num_joints,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        },
        //offset
        {
            .w = 48,
            .h = 48,
            .c =2 * decoder_config.num_joints,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        }
    };
    // set output buffer
    float * output_buffer1 = (float *)malloc(outputs[0].c * outputs[0].h * outputs[0].w * sizeof(float));
    if(! output_buffer1)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[0].data = output_buffer1;

    float * output_buffer2 = (float *)malloc(outputs[1].c * outputs[1].h * outputs[1].w * sizeof(float));
    if(! output_buffer2)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[1].data = output_buffer2;

    float * output_buffer3 = (float *)malloc(outputs[2].c * outputs[2].h * outputs[2].w * sizeof(float));\
    if(! output_buffer3)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[2].data = output_buffer3;

    float * output_buffer4 = (float *)malloc(outputs[3].c * outputs[3].h * outputs[3].w * sizeof(float));
    if(! output_buffer4)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[3].data = output_buffer4;

    //model  create / init / load
    libmaix_nn_t * nn = libmaix_nn_create();
    if(! nn)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn object create fail\n");
        goto end;
    }

    //init
    err = nn->init(nn);
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //load
    err = nn->load(nn , &model , &param);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    LIBMAIX_DEBUG_PRINTF("start loop\n");
    while(!program_exit)
    {
        CALC_FPS("test")
        //get camera
        err = cam->capture_image(cam, &img);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        err = cam2->capture_image(cam2, &show);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }

        input.data = (uint8_t*)img->data;

        err = nn->forward(nn , &input , outputs);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix loc model  forward fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        pose_decoder->decode(pose_decoder , outputs , &pose_result);

    }





end:
    if (cam)
    {
        LIBMAIX_DEBUG_PRINTF("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    if (pose_decoder)
    {
        pose_decoder->deinit(pose_decoder);
        LIBMAIX_DEBUG_PRINTF("--loc decoder destory\n");
        libmaix_nn_decoder_destroy(&pose_decoder);
    }
    if (nn)
    {
        libmaix_nn_destroy(&nn);
    }
    if (show)
    {
        libmaix_image_destroy(&show);
    }
    if (img)
    {
        libmaix_image_destroy(&img);
    }

    LIBMAIX_DEBUG_PRINTF("--nn module deinit\n");
    libmaix_nn_module_deinit();
    LIBMAIX_DEBUG_PRINTF("--image module deinit\n");
    libmaix_image_module_deinit();
    LIBMAIX_DEBUG_PRINTF("--cam module deinit\n");
    libmaix_camera_module_deinit();
}