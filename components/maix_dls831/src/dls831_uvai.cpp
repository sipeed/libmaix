
#include "dls831_uvai.hpp"

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

using namespace cv;
using namespace std;

extern "C"
{
    #include "imlib.h"

    dls831_uv _dls831_, *dls831 = &_dls831_;

    // static struct timeval old, now;

    uint32_t dls831_get_ms()
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

    // =========================================================================================
    /*
    static struct _dls831_ai_
    {
        const char *model_path_param = "/home/res/yolo2_face_int8.param";
        const char *model_path_bin = "/home/res/yolo2_face_int8.bin";
        const char *inputs_names[1] = {"input0"};
        const char *outputs_names[1] = {"output0"};
        const float opt_param_mean = 127.5;
        const float opt_param_norm = 0.0078125;
        libmaix_nn_layer_t input = {
            .w = 224,
            .h = 224,
            .c = 3,
            .dtype = LIBMAIX_NN_DTYPE_UINT8,
        };
        libmaix_nn_layer_t out_fmap = {
            .w = 7,
            .h = 7,
            .c = 30,
            .dtype = LIBMAIX_NN_DTYPE_FLOAT,
        };
        libmaix_nn_t *nn;
        libmaix_nn_model_path_t model_path;
        libmaix_nn_opt_param_t opt_param;
        // -------------- yolo2 decode -----------------------
        const char *labels[1] = {"face"};
        const float anchors[10] = {1.19, 1.98, 2.79, 4.59, 4.53, 8.92, 8.06, 5.29, 10.32, 10.65};
        libmaix_nn_decoder_t *yolo2_decoder;
        libmaix_nn_decoder_yolo2_result_t yolo2_result;
        libmaix_nn_decoder_yolo2_config_t yolo2_config = {
            .classes_num = sizeof(labels) / sizeof(anchors[0]),
            .threshold = 0.5,
            .nms_value = 0.3,
            .anchors_num = (sizeof(anchors) / sizeof(anchors[0])) / 2,
            .anchors = (float *)anchors,
            .net_in_width = 224,
            .net_in_height = 224,
            .net_out_width = 7,
            .net_out_height = 7,
            .input_width = 224,
            .input_height = 224};
    } _dls831_ai, *dls831_ai = &_dls831_ai;

    static int max_index(float *a, int n)
    {
        int i, max_i = 0;
        float max = a[0];

        for (i = 1; i < n; ++i)
        {
            if (a[i] > max)
            {
                max = a[i];
                max_i = i;
            }
        }
        return max_i;
    }

    static void libmaix_nn_decoder_yolo2_draw(_dls831_ai_ *self, struct libmaix_nn_decoder *obj, libmaix_nn_decoder_yolo2_result_t *result)
    {
        region_layer_t *rl = (region_layer_t *)obj->data;
        char *label = NULL;
        uint32_t image_width = rl->config->input_width;
        uint32_t image_height = rl->config->input_height;
        float threshold = rl->config->threshold;
        libmaix_nn_decoder_yolo2_box_t *boxes = result->boxes;

        for (int i = 0; i < result->boxes_num; ++i)
        {
            int class_id = max_index(rl->probs[i], rl->config->classes_num);
            float prob = result->probs[i][class_id];
            if (prob > threshold)
            {
                libmaix_nn_decoder_yolo2_box_t *b = boxes + i;
                uint32_t x = b->x * image_width - (b->w * image_width / 2);
                uint32_t y = b->y * image_height - (b->h * image_height / 2);
                uint32_t w = b->w * image_width;
                uint32_t h = b->h * image_height;
                printf("%d %d %d %d %d %f %s\n", x, y, w, h, prob, self->labels[class_id]);
            }
        }
    }

    int dls831_ai_load(_dls831_ai_ *self)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;

        self->opt_param.awnn.input_names = (char **)self->inputs_names;
        self->opt_param.awnn.output_names = (char **)self->outputs_names;
        self->opt_param.awnn.input_num = sizeof(self->inputs_names) / sizeof(self->inputs_names[0]);
        self->opt_param.awnn.output_num = sizeof(self->outputs_names) / sizeof(self->outputs_names[0]);

        self->opt_param.awnn.mean[0] = self->opt_param_mean;
        self->opt_param.awnn.mean[1] = self->opt_param.awnn.mean[0];
        self->opt_param.awnn.mean[2] = self->opt_param.awnn.mean[0];

        self->opt_param.awnn.norm[0] = self->opt_param_norm;
        self->opt_param.awnn.norm[1] = self->opt_param.awnn.norm[0];
        self->opt_param.awnn.norm[2] = self->opt_param.awnn.norm[0];

        self->model_path.awnn.param_path = (char *)self->model_path_param;
        self->model_path.awnn.bin_path = (char *)self->model_path_bin;

        self->input.need_quantization = true;

        self->out_fmap.data = (float *)malloc(self->out_fmap.w * self->out_fmap.h * self->out_fmap.c * sizeof(float));
        if (!self->out_fmap.data)
        {
            LIBMAIX_INFO_PRINTF("no memory!!!\n");
            return -1;
        }

        self->input.buff_quantization = (uint8_t *)malloc(self->input.w * self->input.h * self->input.c);
        if (!self->input.buff_quantization)
        {
            LIBMAIX_INFO_PRINTF("no memory!!!\n");
            return -1;
        }

        LIBMAIX_INFO_PRINTF("-- nn create\n");
        self->nn = libmaix_nn_create();
        if (!self->nn)
        {
            LIBMAIX_INFO_PRINTF("libmaix_nn object create fail\n");
            return -1;
        }
        LIBMAIX_INFO_PRINTF("-- nn object init\n");
        err = self->nn->init(self->nn);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_INFO_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
            return -1;
        }
        LIBMAIX_INFO_PRINTF("-- nn object load model\n");
        err = self->nn->load(self->nn, &self->model_path, &self->opt_param);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_INFO_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
            return -1;
        }

        LIBMAIX_INFO_PRINTF("-- yolo2 decoder create\n");
        self->yolo2_decoder = libmaix_nn_decoder_yolo2_create(libmaix_nn_decoder_yolo2_init,
                                                              libmaix_nn_decoder_yolo2_deinit,
                                                              libmaix_nn_decoder_yolo2_decode);
        if (!self->yolo2_decoder)
        {
            LIBMAIX_INFO_PRINTF("no mem\n");
            return -1;
        }
        LIBMAIX_INFO_PRINTF("-- yolo2 decoder init\n");
        err = self->yolo2_decoder->init(self->yolo2_decoder, (void *)&self->yolo2_config);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_INFO_PRINTF("decoder init error:%d\n", err);
            return -1;
        }

        LIBMAIX_INFO_PRINTF("nn_gestures_app_load");
    }

    int dls831_ai_exit(_dls831_ai_ *self)
    {
        if (self->yolo2_decoder)
        {
            self->yolo2_decoder->deinit(self->yolo2_decoder);
            libmaix_nn_decoder_yolo2_destroy(&self->yolo2_decoder);
            self->yolo2_decoder = NULL;
        }
        if (self->input.buff_quantization)
        {
            free(self->input.buff_quantization);
            self->input.buff_quantization = NULL;
        }
        if (self->out_fmap.data)
        {
            free(self->out_fmap.data);
            self->out_fmap.data = NULL;
        }
        if (self->nn)
        {
            libmaix_nn_destroy(&self->nn);
        }
    }

    int dls831_ai_loop(_dls831_ai_ *self)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        libmaix_image_t *ai_rgb = NULL;
        if (dls831->ai && LIBMAIX_ERR_NONE == dls831->ai->capture_image(dls831->ai, &ai_rgb))
        {
            dls831->sensor_time = dls831_get_ms();
            // LIBMAIX_INFO_PRINTF("ai_rgb: %p, %d, %d\r\n", ai_rgb, ai_rgb->width, ai_rgb->height);
            // cv::Mat rgb(ai_rgb->height, ai_rgb->width, CV_8UC3, ai_rgb->data);

            self->input.data = ai_rgb->data;
            err = self->nn->forward(self->nn, &self->input, &self->out_fmap);
            if (err != LIBMAIX_ERR_NONE)
            {
                printf("libmaix_nn forward fail: %s\n", libmaix_get_err_msg(err));
            }

            err = self->yolo2_decoder->decode(self->yolo2_decoder, &self->out_fmap, (void *)&self->yolo2_result);
            if (err != LIBMAIX_ERR_NONE)
            {
                printf("yolo2 decode fail: %s\n", libmaix_get_err_msg(err));
            }

            if (self->yolo2_result.boxes_num > 0)
            {
                // libmaix_nn_decoder_yolo2_draw(self, self->yolo2_decoder, &self->yolo2_result);
                LIBMAIX_INFO_PRINTF("yolo2_result.boxes_num %d", self->yolo2_result.boxes_num);
            }
        }
        CALC_FPS("ai");
        return 0;
    }
    */
    // =========================================================================================

    void dls831_vi_open()
    {
        dls831->vi = libmaix_cam_create(0, dls831->vi_w, dls831->vi_h, 0, 0);
        if (NULL == dls831->vi)
            return;
        dls831->vi->start_capture(dls831->vi);

        dls831->ai = libmaix_cam_create(1, dls831->ai_w, dls831->ai_h, 0, 0);
        if (NULL == dls831->ai)
            return;
        dls831->ai->start_capture(dls831->ai);
    }

    void dls831_vi_stop()
    {
        if (NULL != dls831->vi)
            libmaix_cam_destroy(&dls831->vi);

        if (NULL != dls831->ai)
            libmaix_cam_destroy(&dls831->ai);
    }

    void dls831_vi_load()
    {
        LIBMAIX_DEBUG_PRINTF("dls831_vi_load");
        libmaix_camera_module_init();
        libmaix_image_module_init();
        libmaix_nn_module_init();

        dls831->vi_w = dls831_vi_w, dls831->vi_h = dls831_vi_h;
        dls831->ai_w = dls831_ai_w, dls831->ai_h = dls831_ai_h;
        dls831->ui_w = dls831_ui_w, dls831->ui_h = dls831_ui_h;

        dls831_vi_open();

// dls831->ai_rgb = (uint8_t *)malloc(dls831->ai_w * dls831->ai_h * 3);
// if (NULL == dls831->ai_rgb)
//     return;
#if USE_UI
        dls831->vo = libmaix_vo_create(dls831->ui_w, dls831->ui_h, 0, 0, dls831->ui_w, dls831->ui_h);
        if (NULL == dls831->vo)
            return;
#else
        dls831->vi_yuv = (uint8_t *)g2d_allocMem(dls831->vi_w * dls831->vi_h * 3 / 2);
        if (NULL == dls831->vi_yuv)
            return;
#endif
        // dls831->ui_rgba = libmaix_image_create(dls831->ui_w, dls831->ui_h, LIBMAIX_IMAGE_MODE_RGBA8888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, false);
        // if (NULL == dls831->ui_rgba)
        //     return;

        dls831->vi_th_ms = 20; // 30ms 33fps for vi & hw

        fb_realloc_init1(4 * 1024 * 1024);
    }

    void dls831_vi_exit()
    {
        fb_alloc_close0();

        dls831_vi_stop();

        // if (NULL != dls831->ai_rgb)
        //     free(dls831->ai_rgb), dls831->ai_rgb = NULL;

#if USE_UI
        if (NULL != dls831->vo)
            libmaix_vo_destroy(&dls831->vo), dls831->vo = NULL;
#else
        if (NULL != dls831->vi_yuv)
            g2d_freeMem(dls831->vi_yuv), dls831->vi_yuv = NULL;
#endif

        // if (dls831->ui_rgba)
        //     libmaix_image_destroy(&dls831->ui_rgba);

        libmaix_nn_module_deinit();
        libmaix_image_module_deinit();
        libmaix_camera_module_deinit();
        LIBMAIX_DEBUG_PRINTF("dls831_vi_exit");
    }

    enum adaptiveMethod
    {
        meanFilter,
        gaaussianFilter,
        medianFilter
    };

    void AdaptiveThreshold(cv::Mat &src, cv::Mat &dst, double Maxval, int Subsize, double c, adaptiveMethod method = meanFilter)
    {

        if (src.channels() > 1)
            cv::cvtColor(src, src, cv::COLOR_RGB2GRAY);

        cv::Mat smooth;
        switch (method)
        {
        case meanFilter:
            cv::blur(src, smooth, cv::Size(Subsize, Subsize)); // 均值滤波
            break;
        case gaaussianFilter:
            cv::GaussianBlur(src, smooth, cv::Size(Subsize, Subsize), 0, 0); // 高斯滤波
            break;
        case medianFilter:
            cv::medianBlur(src, smooth, Subsize); // 中值滤波
            break;
        default:
            break;
        }
        smooth = smooth - c;
        src.copyTo(dst);
        for (int r = 0; r < src.rows; ++r)
        {
            const uchar *srcptr = src.ptr<uchar>(r);
            const uchar *smoothptr = smooth.ptr<uchar>(r);
            uchar *dstptr = dst.ptr<uchar>(r);
            for (int c = 0; c < src.cols; ++c)
            {
                if (srcptr[c] > smoothptr[c])
                {
                    dstptr[c] = Maxval;
                }
                else
                    dstptr[c] = 0;
            }
        }
    }

    Point2f getCrossPoint(Vec4i LineA, Vec4i LineB)
    {
        double ka, kb;
        ka = (double)(LineA[3] - LineA[1]) / (double)(LineA[2] - LineA[0]); // 求出LineA斜率
        kb = (double)(LineB[3] - LineB[1]) / (double)(LineB[2] - LineB[0]); // 求出LineB斜率

        Point2f crossPoint;
        crossPoint.x = (ka * LineA[0] - LineA[1] - kb * LineB[0] + LineB[1]) / (ka - kb);
        crossPoint.y = (ka * kb * (LineA[0] - LineB[0]) + ka * LineB[1] - kb * LineA[1]) / (ka - kb);
        return crossPoint;
    }

    bool isPointOnLine(Point2f point, Vec4i line)
    {
        double k, b;
        k = (double)(line[3] - line[1]) / (double)(line[2] - line[0]); // 求出LineA斜率
        b = line[1] - k * line[0];
        return (point.y - k * point.x + b) < 2;
    }

    double getAngle(Vec4i LineA, Vec4i LineB)
    {
        double ka, kb;
        ka = (double)(LineA[3] - LineA[1]) / (double)(LineA[2] - LineA[0]); // 求出LineA斜率
        kb = (double)(LineB[3] - LineB[1]) / (double)(LineB[2] - LineB[0]); // 求出LineB斜率

        return atan((ka - kb) / (1 + ka * kb)) * 180 / CV_PI;
    }

    void dls831_vi_loop()
    {
        // CALC_FPS("dls831_vi_loop");
        // LIBMAIX_INFO_PRINTF("dls831_vi_loop");

        cap_set();
        void *ui_vo = dls831->vo->get_frame(dls831->vo, 9);
        if (ui_vo != NULL)
        {
            uint32_t *ui_phy = NULL, *ui_vir = NULL;
            dls831->vo->frame_addr(dls831->vo, ui_vo, &ui_vir, &ui_phy);
            cv::Mat bgra(dls831->ui_h, dls831->ui_w, CV_8UC4, (unsigned char *)ui_vir[0]);
            bgra.setTo(cv::Scalar(0, 0, 0, 0));

            void *vi_vo = dls831->vo->get_frame(dls831->vo, 0);
            if (vi_vo != NULL)
            {
                uint32_t *vi_phy = NULL, *vi_vir = NULL;
                dls831->vo->frame_addr(dls831->vo, vi_vo, &vi_vir, &vi_phy);

                if (dls831->vi && LIBMAIX_ERR_NONE == dls831->vi->capture(dls831->vi, (unsigned char *)vi_vir[0]))
                {
                    cv::Mat gray(dls831->vi_h, dls831->vi_w, CV_8UC1, (unsigned char *)vi_vir[0]);

                    image_t imlib_img, *img = &imlib_img;
                    {
                        img->w = gray.cols;
                        img->h = gray.rows;
                        img->data = (uint8_t *)gray.data;
                        img->size = img->w * img->h * 1;
                        img->is_data_alloc = NULL;
                        img->pixfmt = PIXFORMAT_GRAYSCALE;
                    }

                    float strength = 2.0f;
                    float zoom = 1.0f;
                    float x_corr = 0.0f;
                    float y_corr = 0.0f;

                    fb_alloc_mark();
                    imlib_lens_corr(img, strength, zoom, x_corr, y_corr);
                    fb_alloc_free_till_mark();

                    // 自适应阈值 提取边缘
                    AdaptiveThreshold(gray, gray, 255, 21, 10, meanFilter);
                    // adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, 2);

                    // 过滤掉干扰线，去 x 和 y 上的干扰线，先调好它，清理脏东西
                    {
                        // bitwise_not(gray, gray);
                        // cv::erode(gray, gray, getStructuringElement(MORPH_RECT, Size(1, 1), Point(-1, -1))); // 腐蚀
                        // bitwise_not(gray, gray);
                        // cv::dilate(gray, gray, getStructuringElement(MORPH_RECT, Size(3, 3))); // 膨胀
                        // cv::erode(gray, gray, getStructuringElement(MORPH_RECT, Size(5, 1))); // 腐蚀
                    }

                    rectangle_t roi;
                    roi.x = 0;
                    roi.y = 0;
                    roi.w = gray.cols;
                    roi.h = gray.rows;


                    {
                        fb_alloc_mark();

                        bool invert = false;
                        unsigned int x_stride = 5;
                        unsigned int y_stride = 5;
                        unsigned int area_threshold = 20;
                        unsigned int pixels_threshold = 20;
                        bool merge = false;
                        int margin = 0;
                        unsigned int x_hist_bins_max = 0;
                        unsigned int y_hist_bins_max = 0;
                        list_t out;

                        list_t gray_line_thresholds;
                        imlib_list_init(&gray_line_thresholds, sizeof(color_thresholds_list_lnk_data_t));
                        color_thresholds_list_lnk_data_t gray_line_threshold = {
                            .LMin = 0,
                            .LMax = 70,
                        };

                        list_push_back(&gray_line_thresholds, &gray_line_threshold);

                        imlib_find_blobs(&out, img, &roi, x_stride, y_stride, &gray_line_thresholds, invert,
                                        area_threshold, pixels_threshold, merge, margin,
                                        NULL, NULL, NULL, NULL, x_hist_bins_max, y_hist_bins_max);

                        list_clear(&gray_line_thresholds);

                        for (size_t i = 0; list_size(&out); i++)
                        {
                            find_blobs_list_lnk_data_t lnk_data;
                            list_pop_front(&out, &lnk_data);

                            rectangle_t *r = &lnk_data.rect;
                            cv::rectangle(bgra, Point(r->x, r->y), Point(r->x + r->w, r->y + r->h), Scalar(0, 0, 255, 255), 2, 8, 0);
                        }

                        list_clear(&out);
                        fb_alloc_free_till_mark();
                    }

                    unsigned int x_stride = 15;
                    unsigned int y_stride = 10;
                    uint32_t threshold = 500;
                    unsigned int theta_margin = 25;
                    unsigned int rho_margin = 25;

                    list_t out;
                    fb_alloc_mark();
                    imlib_find_lines(&out, img, &roi, x_stride, y_stride, threshold, theta_margin, rho_margin);
                    fb_alloc_free_till_mark();

                    vector<Vec4i> lines;
                    for (size_t i = 0; list_size(&out); i++)
                    {
                        find_lines_list_lnk_data_t lnk_data;
                        list_pop_front(&out, &lnk_data);
                        line_t *line = &lnk_data.line;
                        lines.push_back(Vec4i(line->x1, line->y1, line->x2, line->y2));
                    }

                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        Vec4i l = lines[i];
                        cv::line(bgra, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0, 127), 1, LINE_AA);
                    }

                    struct reticle_result
                    {
                        Point2f crossPoint;
                        Vec4i lines[2];
                        double angle;
                    };

                    // 识别线段中的十字线
                    vector<reticle_result> reticle_results;
                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        Vec4i l1 = lines[i];
                        for (size_t j = i + 1; j < lines.size(); j++)
                        {
                            Vec4i l2 = lines[j];
                            Point2f crossPoint = getCrossPoint(l1, l2);
                            if (crossPoint.x > 0 && crossPoint.y > 0)
                            {
                                double angle = getAngle(l1, l2);
                                if (angle > 80 && angle < 100)
                                {
                                    reticle_result reticle_result;
                                    reticle_result.crossPoint = crossPoint;
                                    reticle_result.lines[0] = l1;
                                    reticle_result.lines[1] = l2;
                                    reticle_result.angle = angle;
                                    reticle_results.push_back(reticle_result);
                                }
                            }
                        }
                    }

                    // 绘制十字线
                    for (size_t i = 0; i < reticle_results.size(); i++)
                    {
                        reticle_result reticle_result = reticle_results[i];
                        cv::line(bgra, Point(reticle_result.lines[0][0], reticle_result.lines[0][1]), Point(reticle_result.lines[0][2], reticle_result.lines[0][3]), Scalar(255, 0, 0, 255), 2, LINE_AA);
                        cv::line(bgra, Point(reticle_result.lines[1][0], reticle_result.lines[1][1]), Point(reticle_result.lines[1][2], reticle_result.lines[1][3]), Scalar(255, 0, 0, 255), 2, LINE_AA);
                        cv::circle(bgra, reticle_result.crossPoint, 5, Scalar(0, 0, 255, 255), -1, LINE_AA);
                    }

                    // struct serial_data
                    // {
                    //     uint8_t head;
                    //     uint8_t len;
                    //     uint8_t retain_0;
                    //     uint8_t retain_1;
                    //     uint32_t tm;
                    //     uint32_t id;
                    //     float decision_margin;
                    //     float center[2];
                    //     float points[4][2];
                    //     float rotation[3][3];
                    //     uint8_t retain_2;
                    //     uint8_t retain_3;
                    //     uint8_t sum;
                    //     uint8_t end;
                    // } upload_data = {0x55, sizeof(struct serial_data), 0, 0, dls831_get_ms(), 0, 0, {0, 0}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0, 0, 0, 0x0A};

                    // uint8_t *ptr = (uint8_t *)&upload_data;
                    // // uint8_t sum = 0;
                    // for (int i = 0; i < upload_data.len - 2; i++)
                    // upload_data.sum += ptr[i];
                    // // upload_data.sum = sum;
                    // write(gs831->dev_ttyS, ptr, upload_data.len);

                }

                dls831->vo->set_frame(dls831->vo, vi_vo, 0);
            }

            dls831->vo->set_frame(dls831->vo, ui_vo, 9);
        }
        cap_get("dls831->vi");

        cap_set();
        pthread_mutex_lock(&dls831->vi_mutex);
        libmaix_image_t *ai_rgb = NULL;
        if (LIBMAIX_ERR_NONE == dls831->ai->capture_image(dls831->ai, &ai_rgb))
        {
            // cv::Mat cv_src(ai_rgb->height, ai_rgb->width, CV_8UC3, ai_rgb->data);
            // cv::rectangle(cv_src, cv::Point(24, 24), cv::Point(200, 200), cv::Scalar(255, 0, 0), 5);
            // void *tmp = dls831->vo->get_frame(dls831->vo, 9);
            // if (tmp != NULL)
            // {
            //     uint32_t *phy = NULL, *vir = NULL;
            //     dls831->vo->frame_addr(dls831->vo, tmp, &vir, &phy);
            //     cv::Mat bgra(dls831->ui_h, dls831->ui_w, CV_8UC4, (unsigned char *)vir[0]);
            //     cv::rectangle(bgra, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0, 0, 255, 128), 20);
            //     // cv::Mat cv_dst;
            //     // cv::resize(cv_src, cv_dst, cv::Size(dls831->ui_w, dls831->ui_h));
            //     // cv::cvtColor(cv_dst, bgra, cv::COLOR_RGB2BGRA);
            //     dls831->vo->set_frame(dls831->vo, tmp, 9);
            // }
        }
        pthread_mutex_unlock(&dls831->vi_mutex);
        cap_get("dls831->ai");
    }

    void dls831_write_string_to_file(std::string path, std::string txt)
    {
        // FILE *fp = NULL;
        // fp = fopen(path.c_str(), "w+");
        // if (fp) {
        //     fprintf(fp, "%s", txt.c_str());
        //     fclose(fp);
        // }
        // printf("fp %p\r\n", fp);
        std::ofstream outfile(path);
        outfile << txt;
        outfile.flush();
        outfile.close();
    }

    std::string dls831_read_file_to_string(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    void dls831_save_json_conf(const std::string &path, json5pp::value &cfg)
    {
        auto tmp = cfg.stringify();
        LIBMAIX_INFO_PRINTF("save_json_conf %s\n", tmp.c_str());
        dls831_write_string_to_file(path, tmp);
        system("sync");
    }

    void dls831_load_json_conf(const std::string &path, json5pp::value &cfg, json5pp::value old)
    {
        std::string conf = dls831_read_file_to_string(path);
        try
        {
            cfg = json5pp::parse(conf);
            // {
            //     cfg["last_select"] = cfg["last_select"] + 1;
            //     cfg["language"] = "zh-kz";
            //     std::cout << cfg["last_select"] << std::endl;
            //     std::cout << cfg["language"] << std::endl;
            // }
        }
        catch (json5pp::syntax_error e)
        {
            LIBMAIX_INFO_PRINTF("load_json_conf %s : %s", conf.c_str(), e.what());
            system(string_format("rm -rf %s && sync", path.c_str()).c_str());
            cfg = old;
            dls831_save_json_conf(path, cfg);
        }
        LIBMAIX_INFO_PRINTF("load_json_conf %s\n", conf.c_str());
    }

#include <linux/input.h>
#include "linux/watchdog.h"

    /**
     * @brief 看门狗初始化
     * @details
     * 目前看门狗超时时间只能设置为1、2、3、4、5、6、8、10、12、14、16秒
     * @param [in] feed_time          喂狗时间，单位s
     * @retval
     */
    static void _watchdog_init(int feed_time)
    {
        int res;
        int fd = -1;

        fd = open("/dev/watchdog", O_RDWR);
        if (fd < 0)
        {
            fprintf(stderr, "open %s error:%s\n", "/dev/watchdog", strerror(errno));
            return;
        }

        res = ioctl(fd, WDIOC_SETTIMEOUT, &feed_time);
        if (res < 0)
        {
            fprintf(stderr, "watchdog set timeout error\n");
            close(fd);
            return;
        }

        res = ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
        if (res < 0)
        {
            fprintf(stderr, "watchdog enable error\n");
            close(fd);
            return;
        }

        res = close(fd);
        if (res < 0)
        {
            fprintf(stderr, "close %s error:%s\n", "/dev/watchdog", strerror(errno));
            return;
        }
    }

    static void _watchdog_feed(void)
    {
        int res;
        int fd = -1;

        fd = open("/dev/watchdog", O_RDWR);
        if (fd < 0)
        {
            fprintf(stderr, "open %s error:%s\n", "/dev/watchdog", strerror(errno));
            return;
        }

        res = ioctl(fd, WDIOC_KEEPALIVE, 0);
        if (res < 0)
        {
            fprintf(stderr, "watchdog feed error\n");
            close(fd);
            return;
        }

        res = close(fd);
        if (res < 0)
        {
            fprintf(stderr, "close %s error:%s\n", "/dev/watchdog", strerror(errno));
            return;
        }
    }

    void chk831_write_string_to_file(std::string path, std::string txt)
    {
        std::ofstream outfile(path);
        outfile << txt;
        outfile.flush();
        outfile.close();
    }

    std::string chk831_read_file_to_string(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    void chk831_save_json_conf(const std::string &path, json5pp::value &cfg)
    {
        auto tmp = cfg.stringify();
        LIBMAIX_INFO_PRINTF("save_json_conf %s\n", tmp.c_str());
        chk831_write_string_to_file(path, tmp);
        system("sync");
    }

    void chk831_load_json_conf(const std::string &path, json5pp::value &cfg, json5pp::value old)
    {
        std::string conf = chk831_read_file_to_string(path);
        try
        {
            cfg = json5pp::parse(conf);
        }
        catch (json5pp::syntax_error e)
        {
            LIBMAIX_INFO_PRINTF("load_json_conf %s : %s", conf.c_str(), e.what());
            system(string_format("rm -rf %s && sync", path.c_str()).c_str());
            cfg = old;
            chk831_save_json_conf(path, cfg);
        }
        LIBMAIX_INFO_PRINTF("load_json_conf %s\n", conf.c_str());
    }

    void maix_dls831_main(int argc, char *argv[])
    {
        chk831_load_json_conf(dls831->config_file, dls831->config_json, json5pp::object({
                                                                         {"last_select", 0},
                                                                     }));

        void dls831_ctrl_load();
        void dls831_home_load();
        void dls831_ctrl_loop();
        void dls831_home_loop();
        void dls831_ctrl_exit();
        void dls831_home_exit();

        pthread_mutex_init(&dls831->vi_mutex, NULL);
        pthread_mutex_init(&dls831->ai_mutex, NULL);
        pthread_mutex_init(&dls831->ui_mutex, NULL);

        dls831->exit = 0;

        dls831_vi_load();

        dls831_ctrl_load();
        // dls831_home_load();
        dls831->start = time(NULL);
        while (dls831->exit == 0)
        {
            dls831->old = dls831_get_ms();
            CALC_FPS("vi");
            dls831_vi_loop();
            dls831_ctrl_loop();
            // dls831_home_loop();
            // dls831_ui_loop();
            // dls831_ai_loop(dls831_ai);
            // 暂时不使用重启程序，目前测试出现概率很低
            //if (time(NULL) - dls831->start > 5 && dls831_get_ms() - dls831->old > 100) exit(0); // 低于 10 fps 自杀
            // msleep(dls831->vi_th_ms); // 30ms 33fps for vi & hw

            // {
            //     extern uint8_t _set_rgbs_color(uint8_t *color_buf);
            //     __uint8_t rgb[6] = {64,0,0,64,0,0};
            //     _set_rgbs_color(rgb);
            // }
        }
        // dls831_home_exit();
        dls831_ctrl_exit();
        // dls831_ai_exit(dls831_ai);

        dls831_vi_exit();
        dls831->exit = 1;
        chk831_save_json_conf(dls831->config_file, dls831->config_json);
    }
}
