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
    LIBMAIX_INFO_PRINTF("--image module init");
    libmaix_image_module_init();
    LIBMAIX_INFO_PRINTF("--nn module init");
    libmaix_nn_module_init();
    LIBMAIX_INFO_PRINTF("--cam module init");
    libmaix_camera_module_init();

    // process states
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    //input config
    int input_h = 192 , input_w = 192;

    // image object create
    LIBMAIX_DEBUG_PRINTF();
    libmaix_image_t *img = libmaix_image_create(input_w, input_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    LIBMAIX_DEBUG_PRINTF();
    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);

    //camera object create1
    libmaix_cam_t *cam = libmaix_cam_create(0, input_w, input_h, 1, 1);
    LIBMAIX_DEBUG_PRINTF();
    libmaix_cam_t *cam2 = libmaix_cam_create(1, disp->width, disp->height, 0, 0);
    LIBMAIX_DEBUG_PRINTF();
    if (!cam)
    {
        LIBMAIX_ERROR_PRINTF("create cam fail\n");
    }
    err = cam->start_capture(cam);
    LIBMAIX_DEBUG_PRINTF();
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_ERROR_PRINTF("cam capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    err = cam2->start_capture(cam2);
    LIBMAIX_DEBUG_PRINTF();
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_ERROR_PRINTF("cam2 capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //model config
    LIBMAIX_DEBUG_PRINTF();
    libmaix_nn_model_path_t model = {
        .awnn.bin_path = "model_int8.bin",
        .awnn.param_path = "model_int8.param",
    };

    // char * center_weight = "center_weight.bin";
    // char * range_weight_x = "range_weight_x.bin";
    // char * range_weight_y = "range_weight_y.bin";


    //model opt params
    char * input_names [] = {"input1"};
    char * output_names []  = {"output1","output2","output3","output4"};

    LIBMAIX_DEBUG_PRINTF();
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
    LIBMAIX_DEBUG_PRINTF();
    libmaix_nn_decoder_pose_config_t decoder_config = {
        .hm_th = 0.1,
        .image_size = 192,
        .num_joints = 17,
        .center_weight = "./center_weight.bin",
        // .range_weight_x = "range_weight_x.bin",
        // .range_weight_y = "range_weight_y.bin",
    };

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
            LIBMAIX_ERROR_PRINTF("no memory!!!\n");
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
        LIBMAIX_ERROR_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[0].data = output_buffer1;

    float * output_buffer2 = (float *)malloc(outputs[1].c * outputs[1].h * outputs[1].w * sizeof(float));
    if(! output_buffer2)
    {
        LIBMAIX_ERROR_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[1].data = output_buffer2;

    float * output_buffer3 = (float *)malloc(outputs[2].c * outputs[2].h * outputs[2].w * sizeof(float));
    if(! output_buffer3)
    {
        LIBMAIX_ERROR_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[2].data = output_buffer3;

    float * output_buffer4 = (float *)malloc(outputs[3].c * outputs[3].h * outputs[3].w * sizeof(float));
    if(! output_buffer4)
    {
        LIBMAIX_ERROR_PRINTF("no memory!!!\n");
        goto end;
    }
    outputs[3].data = output_buffer4;

    //decoder object create and init


    //decoder result object
    libmaix_nn_decoder_pose_result_t pose_result ;

    //model create
    libmaix_nn_t * nn = NULL;
    nn = libmaix_nn_create();
    if(! nn)
    {
        LIBMAIX_ERROR_PRINTF("libmaix_nn object create fail\n");
        goto end;
    }

    ///decoder create
    LIBMAIX_DEBUG_PRINTF();
    libmaix_nn_decoder_t * pose_decoder = libmaix_nn_decoder_pose_create();
    if(!pose_decoder)
    {
        err = LIBMAIX_ERR_NOT_EXEC;
        LIBMAIX_ERROR_PRINTF("pose decoder create fail:%s\n",libmaix_get_err_msg(err));
        return err;
    }

    //model init
    LIBMAIX_DEBUG_PRINTF();
    err = nn->init(nn);
    LIBMAIX_DEBUG_PRINTF();
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_ERROR_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //decoder init
    LIBMAIX_DEBUG_PRINTF();
    err = pose_decoder->init(pose_decoder , &decoder_config);
    LIBMAIX_DEBUG_PRINTF();
    if (err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_ERROR_PRINTF("decoder init  fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //model load
    LIBMAIX_DEBUG_PRINTF();
    err = nn->load(nn , &model , &param);
    LIBMAIX_DEBUG_PRINTF();
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_ERROR_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    LIBMAIX_INFO_PRINTF("start loop\n");
    while(!program_exit)
    {
        CALC_FPS("test")
        //get camera
        err = cam->capture_image(cam, &img);
        LIBMAIX_DEBUG_PRINTF();
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_ERROR_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        err = cam2->capture_image(cam2, &show);
        LIBMAIX_DEBUG_PRINTF();
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_ERROR_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }

        input.data = (uint8_t*)img->data;
        LIBMAIX_DEBUG_PRINTF();

        err = nn->forward(nn , &input , outputs);
        LIBMAIX_DEBUG_PRINTF();
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_ERROR_PRINTF("libmaix model  forward fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        err = pose_decoder->decode(pose_decoder , outputs , &pose_result);
        LIBMAIX_DEBUG_PRINTF();
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_ERROR_PRINTF("libmaix model decode fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        for(int j=0 ; j < decoder_config.num_joints  ; j++)
        {
            int x = (int)pose_result.keypoints[j*2] * disp->width;
            int y = (int)pose_result.keypoints[j*2 +1] * disp->height;
            // libmaix_err_t libmaix_cv_image_draw_circle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness)
            libmaix_cv_image_draw_circle(show , x , y , 2 ,MaixColor(255, 0, 0), 1);
        }
    }
end:
    if (cam)
    {
        LIBMAIX_INFO_PRINTF("--cam destory\n");
        libmaix_cam_destroy(&cam);
    }
    if (pose_decoder)
    {
        pose_decoder->deinit(pose_decoder);
        LIBMAIX_INFO_PRINTF("--decoder destory\n");
        libmaix_nn_decoder_destroy(&pose_decoder);
        LIBMAIX_INFO_PRINTF("--decoder destory done\n");
    }
    if (nn)
    {
        LIBMAIX_DEBUG_PRINTF();
        libmaix_nn_destroy(&nn);
    }
    if (show)
    {
        LIBMAIX_DEBUG_PRINTF();
        libmaix_image_destroy(&show);
        LIBMAIX_DEBUG_PRINTF();
    }
    if (img)
    {
        LIBMAIX_DEBUG_PRINTF();
        libmaix_image_destroy(&img);
        LIBMAIX_DEBUG_PRINTF();
    }

    LIBMAIX_DEBUG_PRINTF("--nn module deinit\n");
    libmaix_nn_module_deinit();
    LIBMAIX_DEBUG_PRINTF("--image module deinit\n");
    libmaix_image_module_deinit();
    LIBMAIX_DEBUG_PRINTF("--cam module deinit\n");
    libmaix_camera_module_deinit();
}

static void handle_signal(int signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
    {
        program_exit = true;
    }
}

int main(int argc , char *argv[])
{
    struct  libmaix_disp * disp = libmaix_disp_create(0);
    if(disp == NULL)
    {
        LIBMAIX_ERROR_PRINTF("create disp object failed\n");
        return -1;
    }
    signal(SIGINT, handle_signal);
    signal(SIGTERM , handle_signal);

    printf("program start\n");
    nn_test(disp);
    printf("program end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}