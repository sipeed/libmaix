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
#include "mdsc.h"
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
#include <string.h>

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

int  nn_test(struct libmaix_disp *disp)
{
    //init module
    printf("--basic module init\n");
    libmaix_image_module_init();
    libmaix_nn_module_init();
    libmaix_camera_module_init();

    struct libmaix_cam *cam = NULL;
    libmaix_image_t* img;
    libmaix_nn_t *nn = NULL;
    float* result = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    //mdsc infomation
    char * mdsc_path = "/root/mdsc/r329_resnet.mdsc";
    ini_info_t ini_info = read_file(mdsc_path);

    //input (single)
    uint32_t res_h  = ini_info.inputs_shape[0][0];
    uint32_t res_w = ini_info.inputs_shape[0][1];
    uint32_t res_c = ini_info.inputs_shape[0][2];

    // create show image
    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!show)
    {
        printf("create RGB image fail\n");
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
        return 0;
    }
    //create camera
    cam = libmaix_cam_create(0, res_w, res_h, 1, 0);
    if (!cam)
    {
        printf("create cam fail\n");
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
        return 0;
    }
    // cam start capture
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
    if (err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail:%s\n", libmaix_get_err_msg(err));
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
        return 0;
    }

    //input layer
    libmaix_nn_layer_t input = {
        .w = res_w,
        .h = res_h,
        .c = res_c,
        .dtype = LIBMAIX_NN_DTYPE_UINT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL};
    // output layer
    libmaix_nn_layer_t out_fmap = {
        .w = ini_info.outputs_shape[0][1],
        .h = ini_info.outputs_shape[0][0],
        .c = ini_info.outputs_shape[0][2],
        .layout = LIBMAIX_IMAGE_LAYOUT_CHW,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL};


    // malloc  output buffer
    float *output_buffer = (float *)malloc(1000 * sizeof(float));
    if (!output_buffer)
    {
        printf("malloc output buffer is faild !!!\n");
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
        return 0;
    }
    // malloc input buffer
    uint8_t *quantize_buffer = (uint8_t *)malloc(input.w * input.h * input.c);
    if (!quantize_buffer)
    {
        printf("malloc input buffer is faild !!!\n");
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
        return 0;
    }
    input.buff_quantization = quantize_buffer;
    out_fmap.data = output_buffer;

    // init model and opt
    // libmaix_nn_model_path_t model_path;
    // libmaix_nn_opt_param_t opt_param;



    if(strcmp (ini_info.model_type, "awnn") == 0)
    {
        libmaix_nn_model_path_t model_path = {
            .awnn.bin_path = ini_info.bin_path,
            .awnn.param_path = ini_info.param_path,
        };
        libmaix_nn_opt_param_t opt_param = {
            .awnn.input_names = ini_info.inputs,
            .awnn.output_names = ini_info.outpus,
            .awnn.input_num = ini_info.input_num,
            .awnn.output_num = ini_info.output_num,
            .awnn.mean = * ini_info.mean[0],
            .awnn.norm = * ini_info.norm[0],
        };
            //create nn  object
        printf("-- nn create\n");
        nn = libmaix_nn_create();
        if (!nn)
        {
            printf("libmaix_nn object create fail\n");
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
            return 0;
        }
        printf("-- nn object init\n");
        err = nn->init(nn);
        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
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
            return  0;
        }

        printf("-- nn object load model\n");

        err = nn->load(nn, &model_path, &opt_param);
        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
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
            return 0;
        }
    }
    else if (strcmp (ini_info.model_type ,"aipu") == 0)
    {
        libmaix_nn_model_path_t model_path = {
            .aipu.model_path = ini_info.bin_path,
        };
        libmaix_nn_opt_param_t opt_param = {
            .aipu.input_names  =  ini_info.inputs,
            .aipu.output_names = ini_info.outpus,
            .aipu.input_num = ini_info.input_num,
            .aipu.output_num = ini_info.output_num,
            .aipu.mean = * ini_info.mean[0],
            .aipu.norm = * ini_info.norm[0],
            .aipu.scale =  *ini_info.ouputs_scale,
        };
        //create nn  object
        printf("-- nn create\n");
        nn = libmaix_nn_create();
        if (!nn)
        {
            printf("libmaix_nn object create fail\n");
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
            return 0;
        }
        printf("-- nn object init\n");
        err = nn->init(nn);
        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
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
            return  0;
        }

        printf("-- nn object load model\n");

        err = nn->load(nn, &model_path, &opt_param);
        if (err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
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
            return 0;
        }
    }
    else{
        printf("this type model is not support \n");
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
        if (show)
        {
            printf("-- caputer destory\n");
            libmaix_image_destroy(&show);
        }
        printf("--image module deinit\n");
        libmaix_nn_module_deinit();
        libmaix_image_module_deinit();
        libmaix_camera_module_deinit();
        return 0;
    }

    // start loop
    while (true)
    {
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

    printf("draw test\n");
    nn_test(disp);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}