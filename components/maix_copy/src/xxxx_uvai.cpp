
#include "xxxx_uvai.hpp"

/*
designed UI > VI > HW > AI , UI ticks 5ms.
┌──────────────────────────────┐
│                              │
│  UI     touch & lvgl > 30fps │
│                              │
│  AI     ai & vision < 30fps  │
│                              │
│  HW     key & serial > 20fps │
│                              │
│  VI     rgb & yuv > 30fps    │
│                              │
└──────────────────────────────┘
*/

extern "C"
{
    xxxx_uv _xxxx_, *xxxx = &_xxxx_;

    // static struct timeval old, now;

    uint32_t xxxx_get_ms()
    {
        static struct timespec tmp;
        clock_gettime(CLOCK_MONOTONIC, &tmp);
        return (tmp.tv_sec * 1000) + (uint32_t)tmp.tv_nsec / 1000000;
    }

    static void cap_set()
    {
        return;
        // gettimeofday(&old, NULL);
    }

    static void cap_get(const char *tips)
    {
        return;
        // gettimeofday(&now, NULL);
        // if (now.tv_usec > old.tv_usec)
        //     printf("%20s - %5ld ms\r\n", tips, (now.tv_usec - old.tv_usec) / 1000);
    }

    unsigned char *g2d_allocMem(unsigned int size);
    int g2d_freeMem(void *vir_ptr);
    unsigned int g2d_getPhyAddrByVirAddr(void *vir_ptr);

    // static struct _xxxx_ai_
    // {
    //     const char *labels[1] = {"face"};
    //     const char *inputs_names[1] = {"input0"};
    //     const char *outputs_names[1] = {"output0"};
    //     float anchors[10] = {1.19, 1.98, 2.79, 4.59, 4.53, 8.92, 8.06, 5.29, 10.32, 10.65};
    //     float *output_buffer;
    //     uint8_t *quantize_buffer;
    //     libmaix_nn_t *nn;
    //     libmaix_nn_model_path_t model_path;
    //     libmaix_nn_opt_param_t opt_param;
    //     libmaix_nn_layer_t input;
    //     libmaix_nn_layer_t out_fmap;
    //     libmaix_nn_decoder_t *yolo2_decoder;
    //     libmaix_nn_decoder_yolo2_config_t yolo2_config;
    //     libmaix_nn_decoder_yolo2_result_t yolo2_result;
    // } _xxxx_ai, *xxxx_ai = &_xxxx_ai;

    // int xxxx_ai_load()
    // {
    //     libmaix_err_t err = LIBMAIX_ERR_NONE;

    //     xxxx_ai->yolo2_config = {
    //         .classes_num = 1,
    //         .threshold = 0.5,
    //         .nms_value = 0.3,
    //         .anchors_num = 5,
    //         .anchors = xxxx_ai->anchors,
    //         .net_in_width = 224,
    //         .net_in_height = 224,
    //         .net_out_width = 7,
    //         .net_out_height = 7,
    //         .input_width = 224,
    //         .input_height = 224
    //     };

    //     printf("xxxx_ai->yolo2_config.classes_num: %d\n", xxxx_ai->yolo2_config.classes_num);
    //     printf("xxxx_ai->yolo2_config.threshold: %f\n", xxxx_ai->yolo2_config.threshold);
    //     printf("xxxx_ai->yolo2_config.nms_value: %f\n", xxxx_ai->yolo2_config.nms_value);
    //     printf("xxxx_ai->yolo2_config.anchors_num: %d\n", xxxx_ai->yolo2_config.anchors_num);
    //     printf("xxxx_ai->yolo2_config.net_in_width: %d\n", xxxx_ai->yolo2_config.net_in_width);
    //     printf("xxxx_ai->yolo2_config.net_in_height: %d\n", xxxx_ai->yolo2_config.net_in_height);
    //     printf("xxxx_ai->yolo2_config.net_out_width: %d\n", xxxx_ai->yolo2_config.net_out_width);
    //     printf("xxxx_ai->yolo2_config.net_out_height: %d\n", xxxx_ai->yolo2_config.net_out_height);
    //     printf("xxxx_ai->yolo2_config.input_width: %d\n", xxxx_ai->yolo2_config.input_width);
    //     printf("xxxx_ai->yolo2_config.input_height: %d\n", xxxx_ai->yolo2_config.input_height);
    //     printf("xxxx_ai->yolo2_config.anchors: %p\n", xxxx_ai->yolo2_config.anchors);
    //     for(int i = 0; i < xxxx_ai->yolo2_config.anchors_num * 2; i++)
    //         printf("xxxx_ai->yolo2_config.anchors[%d]: %f\n", i, xxxx_ai->yolo2_config.anchors[i]);

    //     xxxx_ai->opt_param.awnn.input_names = (char **)xxxx_ai->inputs_names;
    //     xxxx_ai->opt_param.awnn.output_names = (char **)xxxx_ai->outputs_names;
    //     xxxx_ai->opt_param.awnn.input_num = 1;  // len(input_names)
    //     xxxx_ai->opt_param.awnn.output_num = 1; // len(output_names)

    //     xxxx_ai->opt_param.awnn.mean[0] = 127.5;
    //     xxxx_ai->opt_param.awnn.mean[1] = xxxx_ai->opt_param.awnn.mean[0];
    //     xxxx_ai->opt_param.awnn.mean[2] = xxxx_ai->opt_param.awnn.mean[0];

    //     xxxx_ai->opt_param.awnn.norm[0] = 0.0078125;
    //     xxxx_ai->opt_param.awnn.norm[1] = xxxx_ai->opt_param.awnn.norm[0];
    //     xxxx_ai->opt_param.awnn.norm[2] = xxxx_ai->opt_param.awnn.norm[0];

    //     xxxx_ai->model_path.awnn.param_path = (char *)"/home/res/yolo2_face_int8.param";
    //     xxxx_ai->model_path.awnn.bin_path = (char *)"/home/res/yolo2_face_int8.bin";

    //     xxxx_ai->input = {
    //         .w = 224,
    //         .h = 224,
    //         .c = 3,
    //         .dtype = LIBMAIX_NN_DTYPE_UINT8,
    //     };
    //     xxxx_ai->input.need_quantization = true;

    //     xxxx_ai->out_fmap = {
    //         .w = 7,
    //         .h = 7,
    //         .c = 30,
    //         .dtype = LIBMAIX_NN_DTYPE_FLOAT,
    //     };

    //     xxxx_ai->output_buffer = (float *)malloc(xxxx_ai->out_fmap.w * xxxx_ai->out_fmap.h * xxxx_ai->out_fmap.c * sizeof(float));
    //     if (!xxxx_ai->output_buffer)
    //     {
    //     LIBMAIX_INFO_PRINTF("no memory!!!\n");
    //     return -1;
    //     }

    //     xxxx_ai->quantize_buffer = (uint8_t *)malloc(xxxx_ai->input.w * xxxx_ai->input.h * xxxx_ai->input.c);
    //     if (!xxxx_ai->quantize_buffer)
    //     {
    //     LIBMAIX_INFO_PRINTF("no memory!!!\n");
    //     return -1;
    //     }

    //     xxxx_ai->out_fmap.data = xxxx_ai->output_buffer;
    //     xxxx_ai->input.buff_quantization = xxxx_ai->quantize_buffer;

    //     LIBMAIX_INFO_PRINTF("-- nn create\n");
    //     xxxx_ai->nn = libmaix_nn_create();
    //     if (!xxxx_ai->nn)
    //     {
    //     LIBMAIX_INFO_PRINTF("libmaix_nn object create fail\n");
    //     return -1;
    //     }
    //     LIBMAIX_INFO_PRINTF("-- nn object init\n");
    //     err = xxxx_ai->nn->init(xxxx_ai->nn);
    //     if (err != LIBMAIX_ERR_NONE)
    //     {
    //     LIBMAIX_INFO_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
    //     return -1;
    //     }
    //     LIBMAIX_INFO_PRINTF("-- nn object load model\n");
    //     err = xxxx_ai->nn->load(xxxx_ai->nn, &xxxx_ai->model_path, &xxxx_ai->opt_param);
    //     if (err != LIBMAIX_ERR_NONE)
    //     {
    //     LIBMAIX_INFO_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
    //     return -1;
    //     }

    //     LIBMAIX_INFO_PRINTF("-- yolo2 decoder create\n");
    //     xxxx_ai->yolo2_decoder = libmaix_nn_decoder_yolo2_create();
    //     if (!xxxx_ai->yolo2_decoder)
    //     {
    //     LIBMAIX_INFO_PRINTF("no mem\n");
    //     return -1;
    //     }
    //     LIBMAIX_INFO_PRINTF("-- yolo2 decoder init\n");
    //     err = xxxx_ai->yolo2_decoder->init(xxxx_ai->yolo2_decoder, (void *)&xxxx_ai->yolo2_config);
    //     if (err != LIBMAIX_ERR_NONE)
    //     {
    //     LIBMAIX_INFO_PRINTF("decoder init error:%d\n", err);
    //     return -1;
    //     }

    //     LIBMAIX_INFO_PRINTF("nn_gestures_app_load");
    // }

    // int xxxx_ai_exit()
    // {
    //     if(xxxx_ai->yolo2_decoder)
    //     {
    //         xxxx_ai->yolo2_decoder->deinit(xxxx_ai->yolo2_decoder);
    //         libmaix_nn_decoder_yolo2_destroy(&xxxx_ai->yolo2_decoder);
    //         xxxx_ai->yolo2_decoder = NULL;
    //     }
    //     if(xxxx_ai->output_buffer)
    //     {
    //         free(xxxx_ai->output_buffer);
    //         xxxx_ai->output_buffer = NULL;
    //     }
    //     if(xxxx_ai->nn)
    //     {
    //         libmaix_nn_destroy(&xxxx_ai->nn);
    //     }
    // }

    // int xxxx_ai_loop()
    // {
    //     libmaix_err_t err = LIBMAIX_ERR_NONE;
    //     libmaix_image_t *ai_rgb = NULL;
    //     if (xxxx->ai && LIBMAIX_ERR_NONE == xxxx->ai->capture_image(xxxx->ai, &ai_rgb))
    //     {
    //         xxxx_ai->input.data = ai_rgb->data;
    //         err = xxxx_ai->nn->forward(xxxx_ai->nn, &xxxx_ai->input, &xxxx_ai->out_fmap);
    //         if(err != LIBMAIX_ERR_NONE)
    //         {
    //             printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
    //         }
    //         err = xxxx_ai->yolo2_decoder->decode(xxxx_ai->yolo2_decoder, &xxxx_ai->out_fmap, (void*)&xxxx_ai->yolo2_result);
    //         if(err != LIBMAIX_ERR_NONE)
    //         {
    //             printf("yolo2 decode fail: %s\n", libmaix_get_err_msg(err));
    //         }
    //         if(xxxx_ai->yolo2_result.boxes_num > 0)
    //         {
    //             // libmaix_nn_decoder_yolo2_draw(xxxx_ai, xxxx_ai->yolo2_decoder, &xxxx_ai->yolo2_result);
    //             LIBMAIX_INFO_PRINTF("yolo2_result.boxes_num %d", xxxx_ai->yolo2_result.boxes_num);
    //         }
    //     }
    // }

    // // =========================================================================================

    void xxxx_vi_open()
    {
        xxxx->vi = libmaix_cam_create(0, xxxx->vi_w, xxxx->vi_h, 0, 0);
        if (NULL == xxxx->vi)
            return;
        xxxx->vi->start_capture(xxxx->vi);

        xxxx->ai = libmaix_cam_create(1, xxxx->ai_w, xxxx->ai_h, 0, 0);
        if (NULL == xxxx->ai)
            return;
        xxxx->ai->start_capture(xxxx->ai);
    }

    void xxxx_vi_stop()
    {
        if (NULL != xxxx->vi)
            libmaix_cam_destroy(&xxxx->vi);

        if (NULL != xxxx->ai)
            libmaix_cam_destroy(&xxxx->ai);
    }

    void xxxx_vi_load()
    {
        LIBMAIX_DEBUG_PRINTF("xxxx_vi_load");
        libmaix_camera_module_init();
        libmaix_image_module_init();
        libmaix_nn_module_init();

        xxxx->vi_w = xxxx_vi_w, xxxx->vi_h = xxxx_vi_h;
        xxxx->ai_w = xxxx_ai_w, xxxx->ai_h = xxxx_ai_h;
        xxxx->ui_w = xxxx_ui_w, xxxx->ui_h = xxxx_ui_h;

        xxxx_vi_open();

        // xxxx->vi_yuv = (uint8_t *)g2d_allocMem(xxxx->vi_w * xxxx->vi_h * 3 / 2);
        // if (NULL == xxxx->vi_yuv)
        //     return;

        // xxxx->ai_rgb = (uint8_t *)malloc(xxxx->ai_w * xxxx->ai_h * 3);
        // if (NULL == xxxx->ai_rgb)
        //     return;

        xxxx->vo = libmaix_vo_create(xxxx->ui_w, xxxx->ui_h, 0, 0, xxxx->ui_w, xxxx->ui_h);
        if (NULL == xxxx->vo)
            return;

        // xxxx->ui_rgba = libmaix_image_create(xxxx->ui_w, xxxx->ui_h, LIBMAIX_IMAGE_MODE_RGBA8888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, false);
        // if (NULL == xxxx->ui_rgba)
        //     return;
    }

    void xxxx_vi_exit()
    {
        xxxx_vi_stop();

        // if (NULL != xxxx->vi_yuv)
        //     g2d_freeMem(xxxx->vi_yuv), xxxx->vi_yuv = NULL;

        // if (NULL != xxxx->ai_rgb)
        //     free(xxxx->ai_rgb), xxxx->ai_rgb = NULL;

        if (NULL != xxxx->vo)
            libmaix_vo_destroy(&xxxx->vo), xxxx->vo = NULL;

        // if (xxxx->ui_rgba)
        //     libmaix_image_destroy(&xxxx->ui_rgba);

        libmaix_nn_module_deinit();
        libmaix_image_module_deinit();
        libmaix_camera_module_deinit();
        LIBMAIX_DEBUG_PRINTF("xxxx_vi_exit");
    }

    void xxxx_vi_loop()
    {
        // CALC_FPS("xxxx_vi_loop");
        // LIBMAIX_INFO_PRINTF("xxxx_vi_loop");

        cap_set();
        void *frame = xxxx->vo->get_frame(xxxx->vo, 0);
        if (frame != NULL)
        {
            uint32_t *phy = NULL, *vir = NULL;
            xxxx->vo->frame_addr(xxxx->vo, frame, &vir, &phy);
            if (xxxx->vi && LIBMAIX_ERR_NONE == xxxx->vi->capture(xxxx->vi, (unsigned char *)vir[0]))
            {
                xxxx->vo->set_frame(xxxx->vo, frame, 0);
            }
        }
        cap_get("xxxx->vi");

        // cap_set();
        // pthread_mutex_lock(&xxxx->vi_mutex);
        // if (LIBMAIX_ERR_NONE == xxxx->ai->capture_image(xxxx->ai, &xxxx->ai_rgb))
        // {
        //     // cv::Mat cv_src(xxxx->ai_rgb->height, xxxx->ai_rgb->width, CV_8UC3, xxxx->ai_rgb->data);
        //     // cv::rectangle(cv_src, cv::Point(24, 24), cv::Point(200, 200), cv::Scalar(255, 0, 0), 5);
        //     // void *tmp = xxxx->vo->get_frame(xxxx->vo, 9);
        //     // if (tmp != NULL)
        //     // {
        //     //     uint32_t *phy = NULL, *vir = NULL;
        //     //     xxxx->vo->frame_addr(xxxx->vo, tmp, &vir, &phy);
        //     //     cv::Mat bgra(xxxx->ui_h, xxxx->ui_w, CV_8UC4, (unsigned char *)vir[0]);
        //     //     cv::rectangle(bgra, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0, 0, 255, 128), 20);
        //     //     // cv::Mat cv_dst;
        //     //     // cv::resize(cv_src, cv_dst, cv::Size(xxxx->ui_w, xxxx->ui_h));
        //     //     // cv::cvtColor(cv_dst, bgra, cv::COLOR_RGB2BGRA);
        //     //     xxxx->vo->set_frame(xxxx->vo, tmp, 9);
        //     // }
        // }
        // pthread_mutex_unlock(&xxxx->vi_mutex);
        // cap_get("xxxx->ai");
    }
    void maix_xxxx_main(int argc, char *argv[])
    {
        void xxxx_ctrl_load();
        void xxxx_ctrl_loop();
        void xxxx_ctrl_exit();

        pthread_mutex_init(&xxxx->vi_mutex, NULL);
        pthread_mutex_init(&xxxx->ai_mutex, NULL);

        xxxx->exit = 0;
        xxxx_vi_load();
        // xxxx_ai_load();
        xxxx_ctrl_load();

        while (xxxx->exit == 0)
        {
            xxxx_vi_loop();
            xxxx_ctrl_loop();
            // xxxx_ui_loop();
            // xxxx_ai_loop();
        }

        xxxx_ctrl_exit();
        // xxxx_ai_exit();
        xxxx_vi_exit();
        xxxx->exit = 1;
    }
}
