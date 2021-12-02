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


static void softmax(int8_t *data, int n , float scale,float* out)
{
    int stride = 1;
    int i;
    // int diff;
    // float e;
    float sum = 0;
    float largest_i = data[0];
    for(i = 0; i < n; ++i)
    {
        out[i] = data[i] / scale;

        if (out[i * stride] > largest_i)
            largest_i = out[i * stride];

    }

    for (i = 0; i < n; ++i)
    {
        float value = expf(out[i * stride] - largest_i);
        sum += value;
        out [i * stride] = value;
    }
    for (i = 0; i < n; ++i)
	{
        out[i * stride] /= sum;
	}

}


void nn_test(struct libmaix_disp* disp)
{
    struct libmaix_cam* cam = NULL;
    // libmaix_image_t* img;

    uint32_t res_w = 224, res_h = 224;
    libmaix_nn_t* nn = NULL;
    // float* result = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;

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

    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_camera_module_init();
    libmaix_nn_module_init();
    

    printf("--create image\n");
    // img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_YUV420SP_NV21, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    // if(!img)
    // {
    //     printf("create yuv image fail\n");
    //     goto end;
    // }
    // libmaix_image_t* rgb_img = libmaix_image_create(res_w, res_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    // if(!rgb_img)
    // {
    //     printf("create rgb image fail\n");
    //     goto end;
    // }
    printf("--create cam\n");
    cam = libmaix_cam_create(0, res_w, res_h, 1, 0);
    if(!cam)
    {
        printf("create cam fail\n");
        goto end;
    }
    printf("--cam start capture\n");
    err = cam->start_capture(cam);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail:%s\n", libmaix_get_err_msg(err));
        goto end;
    }

    printf("--resnet init\n");
    libmaix_nn_model_path_t model_path = {
        // .awnn.param_path = "~/root/models/resnet_awnn.param",
        .normal.model_path = "./resnet.bin",
    };
    libmaix_nn_layer_t input = {
        .w = 224,
        .h = 224,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_UINT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    libmaix_nn_layer_t out_fmap = {
        .w = 1,
        .h = 1,
        .c = 1000,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL
    };


    char* inputs_names[] = {"input0"};
    char* outputs_names[] = {"output0"};
    libmaix_nn_opt_param_t opt_param = {
        .awnn.input_names             = inputs_names,
        .awnn.output_names            = outputs_names,
        .awnn.input_num               = 1,              // len(input_names)
        .awnn.output_num              = 1,              // len(output_names)
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.00784313725490196, 0.00784313725490196, 0.00784313725490196},
    };
    int8_t* output_buffer = (int8_t * )malloc(1000 * sizeof(int8_t));
    if(!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    int8_t* quantize_buffer = (int8_t*)malloc(input.w * input.h * input.c);
    if(!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }

    // out_fmap.data = output_buffer;
    // input.data = quantize_buffer;


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

    libmaix_image_t *show = libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    
    
    while(1)
    {
        libmaix_image_t* img = NULL;
        // CALC_TIME_START();
        // printf("--cam capture\n");
        err = cam->capture_image(cam, &img);

        // err = libmaix_cv_image_draw_image_save(img,"./img/before.jpg");
        
        err = libmaix_cv_image_resize(img,240,240,&show);
        // printf("w %d h %d p %d \r\n", img->width, img->height, img->mode);
    
        /*compare the different*/
        uint8_t * pixels =(uint8_t *) (img->data);
        int8_t * quant_data = (int8_t *)malloc(sizeof(int8_t) * (img->width * img->height * img->mode));
        for(int i = 0;i < (img->mode * img->height * img->width);i++)
        {
            // printf("%d\t",pixels[i]);
            quant_data[i] = pixels[i] - 127 ;
            // printf("%d\n",pixels[i]);
        }
        disp->draw_image(disp,show);
        free(img->data);
        img->data = (int8_t *)quant_data;
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
                printf("capture fail, error code: %s\n", libmaix_get_err_msg(err));
                break;
            }
        
        }

        // CALC_TIME_START();
        // printf("--maix nn forward\n");
        
        // //  try to use a sample img to test this funcuiton 
        // int c = input.c;
        // int h = input.h;
        // int w = input.w;
        // int size = c*h*w;

        // // quantize_buffer = (int8_t*) (rgb_img->data);

        /*read pic data*/
        // char * img_path =  "./input.bin";
        // FILE * fp  = fopen(img_path,"rb");
        // if (fp == NULL)
        // {
        //     printf("--open input file faild \n");
        // }
        // fseek(fp,0L,SEEK_SET);
        // int nread = fread(quantize_buffer , 1 , size ,fp);
        // if (nread !=  size)
        // {
        //     printf("read input file faild\n");
        //     free(quantize_buffer);
        //     break;
        // }
        // fclose (fp);

        
        // printf("--start forward\n");
        CALC_TIME_START(); 

        input.data = (int8_t *)(img->data);
        // input.data = quantize_buffer;
        out_fmap.data =output_buffer;
        err = nn->forward(nn, &input, &out_fmap);


        if(err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            break;
        }
        float max_p = 0;
        int max_idx = 0;
        // uint8_t * output_buffer = (uint8_t *) out_fmap.data ;
        // printf("output first one :%d\n",output_buffer[0]);
        float * prediction  = (float *)malloc(1000*sizeof(float));
        softmax(output_buffer, 1000, 7.539542, prediction);
        for(int i=0; i<1000; ++i)
        {
            if(prediction[i] > max_p)
            {
                max_p = prediction[i];
                max_idx = i;
            }
        }
        printf("%f::%s \n", max_p, labels[max_idx]);
        // printf("before quant :%d\n",output_buffer[max_idx]);
        CALC_TIME_END("maix nn forward");

        // CALC_TIME_START();
        // // printf("--convert test end\n");
        // // libmaix_image_color_t color ={
        // //     .rgb888.r = 255,
        // //     .rgb888.g = 0,
        // //     .rgb888.b = 0
        // // };
        // // char temp_str[100];
        // // snprintf(temp_str, 100, "%f, %s", max_p, labels[max_idx]);
        // // rgb_img->draw_string(rgb_img, temp_str, 4, 4, 16, color, NULL);
        // // disp->draw(disp, rgb_img->data);
        // // CALC_TIME_END("display");
    }

end:
    // if(output_buffer)
    // {
    //     free(output_buffer);
    // }
    if(nn)
    {
        nn->deinit(nn);
        printf("--nn destory\n");
        libmaix_nn_destroy(&nn);
    }
    if(cam)
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

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_create(0);
    if(disp == NULL) {
        printf("creat disp object fail\n");
        return -1;
    }

    

    printf("draw test\n");
    nn_test(disp);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}

