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
#include "libmaix_cv_image.h"
#include "libmaix_nn_decoder_ctc.h"
#include "main.h"
#include "time_utils.h"

#define SAVE_NETOUT 1
#define debug_line  //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)

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
    // init module
    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_nn_module_init();
    libmaix_camera_module_init();

    //define net input config
    int input_w = 94 , input_h = 24;
    int disp_w = disp->width , disp_h = disp->height;

    //define nn object
    libmaix_nn_t * nn = NULL;

    //define state object
    libmaix_err_t err = LIBMAIX_ERR_NONE;

    //define decoder object
    libmaix_nn_decoder_t * decoder = NULL;

    //define result object
    char * result = (char*)malloc(sizeof(char) * 10);
    memset(result , 0 , 10);

    //create image object
    libmaix_image_t* img = libmaix_image_create(input_w, input_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    libmaix_image_t * show  =  libmaix_image_create(disp->width, disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);

    //model path
    libmaix_nn_model_path_t model_path = {
        .awnn.param_path = "./lpr_awnn.param",
        .awnn.bin_path = "./lpr_awnn.bin",
    };

    // decoder config
    char * dict[] = {{"皖", "沪", "津", "渝", "冀", "晋", "蒙", "辽", "吉", "黑",
                    "苏", "浙", "京", "闽", "赣", "鲁", "豫", "鄂", "湘", "粤",
                    "桂", "琼", "川", "贵", "云", "藏", "陕", "甘", "青", "宁",
                    "新", "警", "学",
                    "A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W", "X","Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"}};
    libmaix_nn_decoder_ctc_config_t config ={
        .classes_num = 68,
        .T = 18,
        .N = 1,
        .C = 68,
        .labels = dict
    };

    //init nn input layer
    libmaix_nn_layer_t input = {
        .w = input_w,
        .h = input_h,
        .c = 3,
        .dtype = LIBMAIX_NN_DTYPE_INT8,
        .data = NULL,
        .need_quantization = true,
        .buff_quantization = NULL
    };
    //init nn output layer
    libmaix_nn_layer_t output = {
        .c = config.T,
        .h = config.N,
        .w = config.C,
        .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        .data = NULL,
        .layout = LIBMAIX_NN_LAYOUT_CHW,
    };

    // input && output nod's name
    char* inputs_names[] = {"input0"};
    char* outputs_names[] = {"output0"};

    //init nn forward param
    #ifdef CONFIG_ARCH_V831
    libmaix_nn_opt_param_t opt_param = {
        .awnn.input_names             = inputs_names,
        .awnn.output_names            = outputs_names,
        .awnn.encrypt                 = false,
        .awnn.input_num               = 1,
        .awnn.output_num              = 1,
        .awnn.mean                    = {127.5, 127.5, 127.5},
        // .awnn.norm                    = {0.0078125,0.0078125,0.0078125},
        .awnn.norm                    = {127.5, 127.5, 127.5},
    };
    #endif

    //init input quantized buffer
    int8_t *quantize_buffer = (int8_t *)malloc(input.c * input.h * input.w);
    if(!quantize_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    input.buff_quantization = quantize_buffer;

    // init nn forward output buffer
    float* output_buffer = (float*)malloc(output.c * output.w * output.h * sizeof(float));
    if(!output_buffer)
    {
        printf("no memory!!!\n");
        goto end;
    }
    output.data = output_buffer;


    // create nn object
    printf("-- nn create\n");
    nn = libmaix_nn_create();
    if(!nn)
    {
        printf("libmaix_nn object create fail\n");
        goto end;
    }

    //nn object init
    printf("-- nn object init\n");
    err = nn->init(nn);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    // nn model load
    printf("-- nn object load model\n");
    err = nn->load(nn, &model_path, &opt_param);
    printf("-- nn object load model done \n");
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    //create decoder
    decoder = libmaix_nn_decoder_ctc_create();
    //init decoder
    err = decoder->init(decoder ,&config);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }


    //load image
    printf("-- load input bin file\n");
    libmaix_image_t *template = NULL;
    err = libmaix_cv_image_open_file(&template, "./template.png");
    if (err == LIBMAIX_ERR_NONE) return;

    //nn model forward
    input.data = (uint8_t *)template->data;
    CALC_TIME_START();
    err = nn->forward(nn , &input ,&output);
    CALC_TIME_END("forward");
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }

    // save output feature map
    #if SAVE_NETOUT
    printf("saveing dump\n");
    save_bin("string_lpr.bin" , output.c * output.h * output.w *sizeof(float) , output.data);
    #endif

    //doing ctc decode
    CALC_TIME_START();
    decoder->decode(decoder,&output,result);
    CALC_TIME_END("decode");
    printf("result:%s", result);

end:
    if(output_buffer)
    {
        free(output_buffer);
    }
    if(nn)
    {
        libmaix_nn_destroy(&nn);
    }
    if(decoder)
    {
        decoder->deinit(decoder);
        libmaix_nn_decoder_destroy(&decoder);
    }
    printf("--image module deinit\n");
    libmaix_camera_module_deinit();
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





