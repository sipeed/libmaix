
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "time.h"

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "libmaix_cam.h"
#include "libmaix_image.h"
#include "libmaix_disp.h"
#include "libmaix_cv_image.h"
#include <opencv2/opencv.hpp>

#ifdef CONFIG_IMLIB_ENABLE
#include "imlib.h"
#endif

#define CALC_FPS(tips)                                                                                         \
    {                                                                                                          \
        static int fcnt = 0;                                                                                   \
        fcnt++;                                                                                                \
        static struct timespec ts1, ts2;                                                                       \
        clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
        if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
        {                                                                                                      \
            printf("%s => FPS:%d\n", tips, fcnt);                                                         \
            ts1 = ts2;                                                                                         \
            fcnt = 0;                                                                                          \
        }                                                                                                      \
    }

#include "sys/time.h"

static struct timeval old, now;

static void cap_set()
{
    gettimeofday(&old, NULL);
}

static void cap_get(const char *tips)
{
    gettimeofday(&now, NULL);
    if (now.tv_usec > old.tv_usec)
        printf("%20s - %5ld ms\r\n", tips, (now.tv_usec - old.tv_usec) / 1000);
}

struct
{
    int w0, h0, w1, h1;
    struct libmaix_cam *cam0;
#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
    struct libmaix_cam *cam1;
#endif
    uint8_t *rgb888;

    struct libmaix_disp *disp;

    int running;
} app = {0};

static void app_handlesig(int signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
    {
        app.running = 0;
    }
    // exit(0);
}

int app_init(int cam_w, int cam_h, int cam2_w, int cam2_h)
{

    libmaix_camera_module_init();

    memset(&app, 0, sizeof(app));

    app.w0 = cam_w, app.h0 = cam_h;

    app.cam0 = libmaix_cam_create(0, app.w0, app.h0, 1, 0);
    if (NULL == app.cam0)
        return -1;

#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
    app.w1 = cam2_w, app.h1 = cam2_h;
    app.cam1 = libmaix_cam_create(1, app.w1, app.h1, 1, 0);
    if (NULL == app.cam1)
        return -2;
    app.rgb888 = (uint8_t *)malloc(app.w0 * app.h0 * 3);
    if (app.rgb888 == NULL)
        return -3;
#endif

    app.disp = libmaix_disp_create(0);
    if (NULL == app.disp)
        return -4;
    printf("disp w: %d, h: %d\r\n", app.disp->width, app.disp->height);
    if (app.disp->width == 0 || app.disp->height == 0)
        app.disp->width = app.w0, app.disp->height = app.h0;

#ifdef CONFIG_IMLIB_ENABLE
    imlib_init_all();
#endif

    app.running = 1;

    return 0;
}

void app_exit()
{

    if (NULL != app.cam0)
        libmaix_cam_destroy(&app.cam0);

#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
    if (NULL != app.cam1)
        libmaix_cam_destroy(&app.cam1);
#endif

    if (NULL != app.rgb888)
        free(app.rgb888), app.rgb888 = NULL;
    if (NULL != app.disp)
        libmaix_disp_destroy(&app.disp), app.disp = NULL;

    libmaix_camera_module_deinit();
}

void opencv_ops(cv::Mat &rgb)
{
    cv::Mat gray;
    cv::cvtColor(rgb, gray, cv::COLOR_RGB2GRAY);
    cv::Canny(gray, gray, 100, 255, 3, false);
    cv::cvtColor(gray, rgb, cv::COLOR_GRAY2RGB);
}

#ifdef CONFIG_IMLIB_ENABLE
/**
 * usage refer to:
 *       https://github.com/sipeed/MaixPy3/blob/c6b5c419a9c547f1f42c686020eb0e4cdb3f93cf/ext_modules/_maix_image/py_maix_image.cpp#L105
 *       https://github.com/sipeed/MaixPy3/blob/c6b5c419a9c547f1f42c686020eb0e4cdb3f93cf/ext_modules/_maix_image/_maix_image.cpp#L926
*/
void imlib_ops(libmaix_image_t *img)
{
    image_t *mask_img = NULL;
    image_t img_tmp = {}, *arg_img = &img_tmp;
    arg_img->w = img->width;
    arg_img->h = img->height;
    arg_img->pixels = (uint8_t *)img->data;
    arg_img->pixfmt = PIXFORMAT_RGB888;

    list_t thresholds;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    color_thresholds_list_lnk_data_t tmp_ct;
    tmp_ct.LMin = 50;
    tmp_ct.LMax = 100;
    tmp_ct.AMin = -128;
    tmp_ct.AMax = 127;
    tmp_ct.BMin = -128;
    tmp_ct.BMax = 127;
    list_push_back(&thresholds, &tmp_ct);

    fb_alloc_mark();
    imlib_binary(arg_img, arg_img, &thresholds, false, false, mask_img);
    fb_alloc_free_till_mark();
}
#endif

void app_work()
{
    app.cam0->start_capture(app.cam0);

#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
    app.cam1->start_capture(app.cam1);
#endif

    libmaix_image_t *img_disp = libmaix_image_create(app.disp->width, app.disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!img_disp)
    {
        printf("create image failed\r\n");
        return;
    }

    while (app.running)
    {
        // goal code
        libmaix_image_t *img1 = NULL;
        if ( app.cam0->capture_image(app.cam0, &img1) == LIBMAIX_ERR_NONE)
        {
            // printf("w: %d, h: %d, c: %d \r\n", img1->width, img1->height, img1->mode);

            cv::Mat cv_img(img1->height, img1->width, CV_8UC3, img1->data);
            opencv_ops(cv_img);
#ifdef CONFIG_IMLIB_ENABLE
            imlib_ops(img1);
#endif

            // show image to screen
            if (img1->width == app.disp->width && app.disp->height == img1->height)
            {
                app.disp->draw_image(app.disp, img1);
            }
            else
            {
                    libmaix_cv_image_resize(img1, app.disp->width, app.disp->height, &img_disp);
                    app.disp->draw_image(app.disp, img_disp);
            }
            CALC_FPS("maix_cam 0");
        }
#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
        libmaix_image_t *img2 = NULL;
        if (LIBMAIX_ERR_NONE == app.cam1->capture_image(app.cam1, &img2))
        {
            // printf("w: %d, h: %d, c: %d \r\n", img2->width, img2->height, img2->mode);
            CALC_FPS("maix_cam 1");
        }
#endif
    }
    libmaix_image_destroy(&img_disp);
}

int main(int argc, char **argv)
{
    signal(SIGINT, app_handlesig);
    signal(SIGTERM, app_handlesig);

    libmaix_image_module_init();

    app_init(224, 224, 224, 224);
    app_work();
    app_exit();

    libmaix_image_module_deinit();

    return 0;
}
