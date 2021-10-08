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
#include "libmaix_nn_classifier.h"
#include "main.h"
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "board.h"

#define TEST_IMAGE 0

void nn_test(struct libmaix_disp* disp)
{
#if TEST_IMAGE
    int ret = 0;
#endif
    struct libmaix_cam* cam = NULL;
    libmaix_image_t* img;

    libmaix_nn_t* nn = NULL;
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    libmaix_err_t err0 = LIBMAIX_ERR_NONE;
    void* classifier = NULL;

    uint32_t res_w = 224, res_h = 224;
    int class_num = 3;
    int sample_num = 15;
    int feature_length = 512;

    int i_class_num = 0;
    int i_sample_num = 0;


    printf("--nn module init\n");
    libmaix_nn_module_init();
    printf("--image module init\n");
    libmaix_image_module_init();
    libmaix_camera_module_init();

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
        printf("start capture fail: %s\n", libmaix_get_err_msg(err));
        goto end;
    }
#endif
    printf("--resnet init\n");
    libmaix_nn_model_path_t model_path = {
        .awnn.param_path = "/root/models/resnet18_1000_awnn.param",
        .awnn.bin_path = "/root/models/resnet18_1000_awnn.bin",
    };
    char* inputs_names[] = {"input0"};
    char* outputs_names[] = {"190"};
    libmaix_nn_opt_param_t opt_param = {
        .awnn.input_names             = inputs_names,
        .awnn.output_names            = outputs_names,
        .awnn.input_num               = 1,              // len(input_names)
        .awnn.output_num              = 1,              // len(output_names)
        .awnn.mean                    = {127.5, 127.5, 127.5},
        .awnn.norm                    = {0.0176, 0.0176, 0.0176},
    };
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

    printf("-- classifier init, feature_length: %d, class_num: %d, sample_num: %d\n", feature_length, class_num, sample_num);
    int input_w, input_h;
    if(libmaix_classifier_load(&classifier, "m.classifier", nn, &feature_length, &input_w, &input_h, &class_num, &sample_num) != LIBMAIX_ERR_NONE)
    {
        libmaix_classifier_init(&classifier, nn, feature_length, res_w, res_h, class_num, sample_num);
    }

    printf("-- key init\n");
    board_init();
    printf("-- start loop\n");
    while(1)
    {
#if !TEST_IMAGE
        // printf("--cam capture\n");
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
        err0 = img->convert(img, LIBMAIX_IMAGE_MODE_RGB888, &rgb_img);
        if(err0 != LIBMAIX_ERR_NONE)
        {
            printf("conver to RGB888 fail:%s\r\n", libmaix_get_err_msg(err0));
            continue;
        }
#endif
        int key_v, key_v2;
        get_io(2, 3, &key_v, &key_v2);
        if(key_v == 0)
        {
            if(i_class_num < class_num)
            {
                printf("== record class %d\n", i_class_num);
                libmaix_classifier_add_class_img(classifier, rgb_img, &i_class_num);
                ++i_class_num;
            }
            else if(i_sample_num < sample_num)
            {
                printf("== record sample %d\n", i_sample_num);
                int idx = -1;
                libmaix_classifier_add_sample_img(classifier, rgb_img, &idx);
                ++i_sample_num;
                if(i_sample_num == sample_num)
                {
                    printf("== train ...\n");
                    libmaix_classifier_train(classifier);
                    printf("== train complete\n");
                }
            }
        }
        if(key_v2 == 0)
        {
           int ret = libmaix_classifier_save(classifier, "m.classifier");
           printf("save ret: %d\n", ret);
        }
        float distance = 0;
        int class_id = -1;
        err = libmaix_classifier_predict(classifier, rgb_img, &class_id, &distance);
        if(err != LIBMAIX_ERR_NONE)
        {
            printf("libmaix_classifier_predict error! code: %d\n", err);
        }
        if(class_id >= 0)
        {
            printf("class id: %d, distance: %f\n", class_id, distance);
        }

        disp->draw(disp, rgb_img->data);
#if TEST_IMAGE
        break;
#endif
    }
end:
    if(classifier)
    {
        libmaix_classifier_del(&classifier);
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
    board_deinit();
    libmaix_nn_module_deinit();
    libmaix_image_module_deinit();
    libmaix_camera_module_deinit();
}

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_create();
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

