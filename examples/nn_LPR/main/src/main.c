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
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include "libmaix_cv_image.h"
#include "libmaix_nn_decoder_license_plate_location.h"
#include "libmaix_nn_decoder_ctc.h"
#include "main.h"
#include "time_utils.h"
#include <time.h>

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

// const char * labels [] = {"皖", "沪", "津", "渝", "冀", "晋", "蒙", "辽", "吉", "黑",
//                     "苏", "浙", "京", "闽", "赣", "鲁", "豫", "鄂", "湘", "粤",
//                     "桂", "琼", "川", "贵", "云", "藏", "陕", "甘", "青", "宁",
//                     "新", "警", "学",
//                     "A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W", "X","Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"};

const char * labels [] = {"wan*", "hu*", "jin*", "yu2*", "`ji4*", "jin*", "meng*", "liao*", "ji1*", "hei*",
                    "su*", "zhe*", "jing*", "min*", "gan*", "lu*", "yu4*", "e*", "xiang*", "yue",
                    "guilin*", "qiong*", "chuan*", "gui4*", "yun*", "zang*", "shan*", "gan*", "qing*", "ning*",
                    "xin*", "jing3*", "xue*",
                    "A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W", "X","Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"};

void nn_test(struct libmaix_disp* disp)
{
    // module init
    LIBMAIX_DEBUG_PRINTF("--image module init\n");
    libmaix_image_module_init();
    LIBMAIX_DEBUG_PRINTF("--nn module init\n");
    libmaix_nn_module_init();
    LIBMAIX_DEBUG_PRINTF("--cam module init\n");
    libmaix_camera_module_init();

    // process states
    libmaix_err_t err = LIBMAIX_ERR_NONE;


    //model input config
    int loc_w = 224 , loc_h = 224; //location image WH
    int reg_w = 94 , reg_h = 24; //recognize image WH

    //image  object create
    libmaix_image_t* loc_img = libmaix_image_create(loc_w, loc_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_image_t* reg_img = libmaix_image_create(reg_w, reg_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_image_t * show  =  libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);

    //camera object create
    libmaix_cam_t* cam = libmaix_cam_create(0, loc_w, loc_h, 1, 1);
    libmaix_cam_t *cam2 = libmaix_cam_create(1, disp->width, disp->height, 0, 0);
    if(!cam)
    {
        LIBMAIX_DEBUG_PRINTF("create cam fail\n");
    }

    err = cam->start_capture(cam);
    #ifdef CONFIG_ARCH_V831
    err = cam2->start_capture(cam2);
    #endif
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //model config setting
    libmaix_nn_model_path_t loc_model_path = {
        .awnn.param_path = "/root/C/slim_awnn.param",
        .awnn.bin_path = "/root/C/slim_awnn.bin",
    };

    libmaix_nn_model_path_t reg_model_path = {
        .awnn.param_path = "/root/C/lpr_awnn.param",
        .awnn.bin_path = "/root/C/lpr_awnn.bin",
    };

    // model opt params
        // Loc
    char* loc_inputs_names[] = {"input0"};
    char* loc_outputs_names[] = {"output0", "output1", "output2"};
    libmaix_nn_opt_param_t loc_opt_param = {
        .awnn.input_names             = loc_inputs_names,
        .awnn.output_names            = loc_outputs_names,
        .awnn.encrypt                 = false,
        .awnn.input_num               = 1,
        .awnn.output_num              = 3,
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.0078125,0.0078125,0.0078125},
    };
        // Reg
    char* reg_inputs_names[] = {"input0"};
    char* reg_outputs_names[] = {"output0"};
    libmaix_nn_opt_param_t reg_opt_param = {
        .awnn.input_names             = reg_inputs_names,
        .awnn.output_names            = reg_outputs_names,
        .awnn.encrypt                 = false,
        .awnn.input_num               = 1,
        .awnn.output_num              = 1,
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.0078125,0.0078125,0.0078125},
    };

    //decoder config
        //Loc
    libmaix_nn_decoder_license_plate_location_config_t loc_docoder_config = {
        .nms = 0.2,
        .score_thresh = 0.9,
        .input_w = loc_w,
        .input_h = loc_h,
        .variance = {0.1, 0.2},
        .steps = {8, 16, 32},
        .min_sizes = {12, 24, 48, 96, 192, 320},
    };
        //Reg
    libmaix_nn_decoder_ctc_config_t reg_decoder_config = {
        .T = 18,
        .C = 68,
        .N = 1,
        .classes_num = 68,
        .lpr_max_lenght =8,
    };

    //decoder object create and init
        //Loc
    libmaix_nn_decoder_t* loc_decoder = libmaix_nn_decoder_license_plate_location_create();
    err = loc_decoder->init(loc_decoder, &loc_docoder_config);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    libmaix_nn_decoder_license_plate_location_result_t loc_result;
        //Reg
   libmaix_nn_decoder_t* reg_decoder = libmaix_nn_decoder_ctc_create();
    err = reg_decoder->init(reg_decoder, &reg_decoder_config);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    libmaix_nn_decoder_ctc_result_t reg_result;
    reg_result.no_repeat_idx = (int*)malloc(sizeof(int) * reg_decoder_config.lpr_max_lenght);
    if(!reg_result.no_repeat_idx)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }

    //input layer setting
        //Loc
    libmaix_nn_layer_t loc_input = {
        .w = loc_w,
        .h = loc_h,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    if (loc_input.need_quantization)
    {
        int8_t* loc_quantize_buffer = (int8_t*)malloc(loc_input.w * loc_input.h * loc_input.c);
        if(!loc_quantize_buffer)
        {
            LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
            goto end;
        }
        loc_input.buff_quantization = loc_quantize_buffer;
    }
        //Reg
    libmaix_nn_layer_t reg_input = {
        .w = reg_w,
        .h = reg_h,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    if (reg_input.need_quantization)
    {
        int8_t* reg_quantize_buffer = (int8_t*)malloc(reg_input.w * reg_input.h * reg_input.c);
        if(!reg_quantize_buffer)
        {
            LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
            goto end;
        }
        reg_input.buff_quantization = reg_quantize_buffer;
    }

    //output layer setting
        //Loc
    libmaix_nn_layer_t loc_out_fmap[3] = {
        {
            .w = 4,
            .h = 1,
            .c = loc_docoder_config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        },
        {
            .w = 2,
            .h = 1,
            .c = loc_docoder_config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        },
        {
            .w = 8,
            .h = 1,
            .c = loc_docoder_config.channel_num,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
            .data = NULL,
            .layout = LIBMAIX_NN_LAYOUT_CHW,
        }
    };
    float* output_buffer1 = (float*)malloc(loc_out_fmap[0].c * loc_out_fmap[0].w * loc_out_fmap[0].h * sizeof(float));
    if(!output_buffer1)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    loc_out_fmap[0].data = output_buffer1;

    float* output_buffer2 = (float*)malloc(loc_out_fmap[1].c * loc_out_fmap[1].w * loc_out_fmap[1].h * sizeof(float));
    if(!output_buffer2)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    loc_out_fmap[1].data = output_buffer2;

    float* output_buffer3 = (float*)malloc(loc_out_fmap[2].c * loc_out_fmap[2].w * loc_out_fmap[2].h * sizeof(float));
    if(!output_buffer3)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    loc_out_fmap[2].data = output_buffer3;
        //Reg
    libmaix_nn_layer_t reg_out_fmap = {
        .c = reg_decoder_config.N,
        .h = reg_decoder_config.C,
        .w = reg_decoder_config.T,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL,
        .layout = LIBMAIX_NN_LAYOUT_CHW,
    };
    float* output_buffer = (float*)malloc(reg_out_fmap.c * reg_out_fmap.w * reg_out_fmap.h * sizeof(float));
    if(!output_buffer)
    {
        LIBMAIX_DEBUG_PRINTF("no memory!!!\n");
        goto end;
    }
    reg_out_fmap.data = output_buffer;

    //nn model create | init | load
        //create
    libmaix_nn_t* loc_nn = libmaix_nn_create();
    if(!loc_nn)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn object create fail\n");
        goto end;
    }
    libmaix_nn_t* reg_nn = libmaix_nn_create();
    if(!reg_nn)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn object create fail\n");
        goto end;
    }
        //init
    err = loc_nn->init(loc_nn);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    err = reg_nn->init(reg_nn);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
        //load
    err = loc_nn->load(loc_nn, &loc_model_path, &loc_opt_param);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
    err = reg_nn->load(reg_nn, &reg_model_path, &reg_opt_param);
    if(err != LIBMAIX_ERR_NONE)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    LIBMAIX_DEBUG_PRINTF("start loop\n");
    while (!program_exit)
    {
        CALC_FPS("test");
        //get camera
        err = cam->capture_image(cam, &loc_img);
        if(err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }
        err = cam2->capture_image(cam2, &show);
        if(err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("start capture fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }

        loc_input.data = (uint8_t *)loc_img->data;
        CALC_TIME_START();
        err = loc_nn->forward(loc_nn, &loc_input,loc_out_fmap);
        CALC_TIME_END("loc forward");
        if(err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix loc model  forward fail: %s\n", libmaix_get_err_msg(err));
            goto end;
        }

        loc_decoder->decode(loc_decoder,loc_out_fmap, &loc_result);
        for (int i = 0; i < loc_result.num; ++i)
        {
            int affine_src_pts [6];
            int affine_dst_pts[6] = {reg_w , reg_h , 0 , reg_h , 0,0};

            if (loc_result.plates[i].score > loc_docoder_config.score_thresh)
            {
                int x1 = loc_result.plates[i].box.x * disp->width;
                int y1 = loc_result.plates[i].box.y * disp->height ;
                int x2 = x1 + loc_result.plates[i].box.w * disp->width ;
                int y2 = y1 + loc_result.plates[i].box.h * disp->height;
                int w = loc_result.plates[i].box.w * disp->width ;
                int h = loc_result.plates[i].box.h * disp->height ;

                for (int j = 0; j < 3; ++j)
                {
                    affine_src_pts[j*2] = loc_result.plates[i].points[j * 2] * show->width;
                    affine_src_pts[j*2+1] = loc_result.plates[i].points[j * 2 + 1] * show->height;
                }

                // affine_src_pts[0]  = x2;
                // affine_src_pts[1] =  y2;
                // affine_src_pts[2] = x1;
                // affine_src_pts[3] = y2;
                // affine_src_pts[4] = x1;
                // affine_src_pts[5]  = y1;

                if(x1 < 0 ||  y1 < 0 ||  x1 + w > disp->width || y1 + h > disp->height )
                {
                    continue;
                }

                // libmaix_image_t* crop_temp = libmaix_image_create(w, h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
                // err = libmaix_cv_image_crop(show , x1, y1 , w , h , &crop_temp);
                // err = libmaix_cv_image_resize(crop_temp,  reg_w , reg_h ,&reg_img);

                err =  libmaix_cv_image_affine(show , affine_src_pts , affine_dst_pts ,reg_h ,reg_w ,&reg_img);
                reg_input.data = (uint8_t *)reg_img->data;
                err = reg_nn->forward(reg_nn ,&reg_input, &reg_out_fmap);
                if(err != LIBMAIX_ERR_NONE)
                {
                    LIBMAIX_DEBUG_PRINTF("libmaix loc model  forward fail: %s\n", libmaix_get_err_msg(err));
                    goto end;
                }

                CALC_TIME_START();
                reg_decoder->decode(reg_decoder, &reg_out_fmap, &reg_result);
                CALC_TIME_END("decode LP info");
                char *string = (char *)malloc(20);
                memset(string, 0 ,20);
                for(int i=0 ;i!=reg_result.length ;i++)
                {
                    int idx = (int)reg_result.no_repeat_idx[i];
                    strcat(string , labels[idx]);
                }

                // draw
                for (int j = 0; j < 4; ++j)
                {
                    int x = loc_result.plates[i].points[j * 2] * show->width;
                    int y = loc_result.plates[i].points[j * 2 + 1] * show->height;
                    libmaix_cv_image_draw_rectangle(show, x, y , x, y , MaixColor(0, 255, 23), -1);
                }
                libmaix_cv_image_draw_rectangle(show, x1, y1, x2, y2, MaixColor(255, 0, 0), 1);
                libmaix_cv_image_draw_string(show, x1, y1 , string, 1.5 , MaixColor(255, 0, 0 ), 2);
                 libmaix_cv_image_draw_image(show, 0, 0, reg_img, 1.0);
                free(string);
                // libmaix_image_destroy(&crop_temp);

            }
        }
        disp->draw_image(disp, show);
    }


end:
    if(cam)
    {
         LIBMAIX_DEBUG_PRINTF("--cam destory\n");
         libmaix_cam_destroy(&cam);
    }
    if(loc_decoder)
    {
        loc_decoder->deinit(loc_decoder);
         LIBMAIX_DEBUG_PRINTF("--loc decoder destory\n");
        libmaix_nn_decoder_destroy(&loc_decoder);
    }
    if(reg_decoder)
    {
         LIBMAIX_DEBUG_PRINTF("--reg decoder destory\n");
        reg_decoder->deinit(reg_decoder);
        libmaix_nn_decoder_destroy(&reg_decoder);
    }
    if(reg_result.no_repeat_idx)
    {
        free(reg_result.no_repeat_idx);
        reg_result.no_repeat_idx = NULL;
    }
    if(loc_nn)
    {
        libmaix_nn_destroy(&loc_nn);
    }
    if(reg_nn)
    {
        libmaix_nn_destroy(&reg_nn);
    }
    if(loc_img)
    {
        libmaix_image_destroy(&loc_img);
    }
    if(reg_img)
    {
        libmaix_image_destroy(&reg_img);
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

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_create(0);
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
