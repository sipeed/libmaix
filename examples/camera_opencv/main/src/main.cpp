/**
 * 
 * @author neucrack@sipeed
 * @license MIT
 * 
*/

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

////////// UART(serial) //////////
#include "linux_uart.h"
//////////////////////////////////

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
static int uart_fd = -1;
static char uart_buff[1024] = {0};

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

#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
extern "C"
{
    int AW_MPI_ISP_AE_SetMode(int IspDev, int Value);			// [0:auto, 1:manual]
    int AW_MPI_ISP_AE_SetGain(int IspDev, int Value);			// [0, 65535]
    int AW_MPI_ISP_AE_SetExposure(int IspDev, int Value);		// [0, 65535*16]
}

#endif

static void set_exposure()
{
#ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
    // AW_MPI_ISP_AE_SetMode(0, 0); // auto

    AW_MPI_ISP_AE_SetMode(0, 1);
    AW_MPI_ISP_AE_SetGain(0, 50);
    AW_MPI_ISP_AE_SetExposure(0, 50);
#endif
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

    // set manual exposure here
    // set_exposure();

    app.disp = libmaix_disp_create(0);
    if (NULL == app.disp)
        return -4;
    printf("disp w: %d, h: %d\r\n", app.disp->width, app.disp->height);
    if (app.disp->width == 0 || app.disp->height == 0)
        app.disp->width = app.w0, app.disp->height = app.h0;

#ifdef CONFIG_IMLIB_ENABLE
    imlib_init_all();
#endif

    // uart init
    // /dev/ttyS1, for maix-ii-dock(v831) TX pin is PG6, RX pin is PG7
    uart_t uart_conf;
    uart_conf.baud = 115200;
    uart_conf.data_bits = 8;
    uart_conf.stop_bits = 1;
    uart_conf.parity = 'N';

    uart_fd = linux_uart_init((char*)"/dev/ttyS1", &uart_conf);

    app.running = 1;

    return 0;
}

void app_exit()
{
    // uart deinit
    if(uart_fd > 0)
        linux_uart_deinit(uart_fd);

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
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0, 0);
    // binary image
    cv::Mat binary;
    cv::threshold(blur, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    // invert image
    cv::Mat invert;
    cv::bitwise_not(binary, invert);
    // find contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(invert, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    // get the largets contour
    int largest_area = 0;
    int largest_contour_index = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area > largest_area)
        {
            largest_area = area;
            largest_contour_index = i;
        }
    }
    // 多边形包围最大轮廓
    std::vector<cv::Point> poly;
    cv::approxPolyDP(contours[largest_contour_index], poly, 3, true);
    // 从 poly 找到凸出的点
    std::vector<cv::Point> hull;
    cv::convexHull(poly, hull);


    // find red point on rgb image
    cv::Mat hsv;
    cv::cvtColor(rgb, hsv, cv::COLOR_RGB2HSV);
    cv::Mat mask, mask2;
    cv::inRange(hsv, cv::Scalar(0, 43, 46), cv::Scalar(10, 255, 255), mask);
    cv::inRange(hsv, cv::Scalar(156, 43, 46), cv::Scalar(180, 255, 255), mask2);
    mask = mask | mask2;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    // find biggest contour on mask
    std::vector<std::vector<cv::Point>> contours2;
    std::vector<cv::Vec4i> hierarchy2;
    cv::findContours(mask, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    // get the largets contour
    int largest_area2 = 0;
    int largest_contour_index2 = 0;
    for (size_t i = 0; i < contours2.size(); i++)
    {
        double area = cv::contourArea(contours2[i]);
        if (area > largest_area2)
        {
            largest_area2 = area;
            largest_contour_index2 = i;
        }
    }

    // find green point on rgb image
    cv::Mat mask_green;
    cv::inRange(hsv, cv::Scalar(35, 43, 46), cv::Scalar(77, 255, 255), mask_green);
    cv::morphologyEx(mask_green, mask_green, cv::MORPH_OPEN, kernel);
    // find biggest contour on mask
    std::vector<std::vector<cv::Point>> contours_green;
    std::vector<cv::Vec4i> hierarchy_green;
    cv::findContours(mask_green, contours_green, hierarchy_green, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    // get the largets contour
    int largest_area_green = 0;
    int largest_contour_index_green = 0;
    for (size_t i = 0; i < contours_green.size(); i++)
    {
        double area = cv::contourArea(contours_green[i]);
        if (area > largest_area_green)
        {
            largest_area_green = area;
            largest_contour_index_green = i;
        }
    }

    // draw points
    // if(hull.size() == 4) // 只在有4个点的时候显示
    {
        // 在 rgb 图上画出凸包
        cv::polylines(rgb, hull, true, cv::Scalar(0, 255, 0), 2);
        // 在 rgb 图上画出 hull 点
        for (size_t i = 0; i < hull.size(); i++)
        {
            cv::circle(rgb, hull[i], 5, cv::Scalar(0, 0, 255), 2);
        }
    }

    // get the center point of the largest contour
    cv::Point2f mc, mc_green;
    if(contours2.size() > 0)
    {
        cv::Moments mu = cv::moments(contours2[largest_contour_index2], false);
        mc = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
        cv::circle(rgb, mc, 5, cv::Scalar(255, 0, 255), 2);
    }
    if(contours_green.size() > 0)
    {
        cv::Moments mu = cv::moments(contours_green[largest_contour_index_green], false);
        mc_green = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
        cv::circle(rgb, mc_green, 5, cv::Scalar(255, 255, 0 ), 2);
    }

    // send result(center point, points) to uart
    snprintf(uart_buff, sizeof(uart_buff), "red: %d, %d, green: %d, %d, points: %d, ",
                    (int)mc.x, (int)mc.y, (int)mc_green.x, (int)mc_green.y, (int)hull.size());
    for(size_t i = 0; i < hull.size(); i++)
    {
        snprintf(uart_buff + strlen(uart_buff), sizeof(uart_buff) - strlen(uart_buff), "%d, %d, ", (int)hull[i].x, (int)hull[i].y);
    }
    snprintf(uart_buff + strlen(uart_buff), sizeof(uart_buff) - strlen(uart_buff), "\r\n");
    write(uart_fd, uart_buff, strlen(uart_buff));
}

#ifdef CONFIG_IMLIB_ENABLE
/**
 * usage refer to:
 *       https://github.com/sipeed/MaixPy3/blob/c6b5c419a9c547f1f42c686020eb0e4cdb3f93cf/ext_modules/_maix_image/py_maix_image.cpp#L105
 *       https://github.com/sipeed/MaixPy3/blob/c6b5c419a9c547f1f42c686020eb0e4cdb3f93cf/ext_modules/_maix_image/_maix_image.cpp#L926
*/
void imlib_ops(libmaix_image_t *img)
{
    // image_t *mask_img = NULL;
    // image_t img_tmp = {}, *arg_img = &img_tmp;
    // arg_img->w = img->width;
    // arg_img->h = img->height;
    // arg_img->pixels = (uint8_t *)img->data;
    // arg_img->pixfmt = PIXFORMAT_RGB888;

    // list_t thresholds;
    // list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    // color_thresholds_list_lnk_data_t tmp_ct;
    // tmp_ct.LMin = 50;
    // tmp_ct.LMax = 100;
    // tmp_ct.AMin = -128;
    // tmp_ct.AMax = 127;
    // tmp_ct.BMin = -128;
    // tmp_ct.BMax = 127;
    // list_push_back(&thresholds, &tmp_ct);

    // fb_alloc_mark();
    // imlib_binary(arg_img, arg_img, &thresholds, false, false, mask_img);
    // fb_alloc_free_till_mark();
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
