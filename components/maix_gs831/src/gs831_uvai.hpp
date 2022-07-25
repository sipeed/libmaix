
#ifndef _gs831_uv_
#define _gs831_uv_

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "libmaix_cam.h"
#include "libmaix_image.h"
#include "libmaix_disp.h"
#include "libmaix_nn.h"
#include "libmaix_nn_decoder_yolo2.h"

#include "linux_uart.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include "opencv2/core/types_c.h"

#include "json5pp.hpp"

// << string_format("%d", 202412);
template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args...); // Extra space for \0
    // unique_ptr<char[]> buf(new char[size]);
    char bytes[size];
    snprintf(bytes, size, format.c_str(), args...);
    return std::string(bytes);
}

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

const int gs831_vi_w = 240, gs831_vi_h = 180;
const int gs831_ui_w = 240, gs831_ui_h = 180;

struct gs831_pack_t
{
    uint8_t type;
    std::vector<uint8_t> data;
};

typedef struct
{
    // sys
    const char *config_file = "/root/gs831.json";
    json5pp::value config_json;

    int exit, signal;
    pthread_mutex_t vi_mutex, ai_mutex, ui_mutex;
    // hw
    struct timeval timeout;
    int input_event0, dev_ttyS;
    fd_set readfd;
    std::list<gs831_pack_t> recvPacks;
    // vi
    uint16_t vi_w, vi_h, ai_w, ai_h, ui_w, ui_h;
    libmaix_cam *vi, *ai;
    // uint8_t *vi_yuv;
    // uint8_t *ai_rgb;
    // libmaix_image_t *ai_rgb;
    libmaix_vo *vo;
    // cv::Mat ui_bgra;
    // ui
    pthread_t ui_thread;
    int ui_th_usec, vi_th_usec;
    // ai
    pthread_t ai_thread;
    int ai_th_usec, ai_th_id;
    volatile int ai_th_keep;
} gs831_uv;

typedef struct _gs831_home_app_
{
    int (*load)(struct _gs831_home_app_ *);
    int (*loop)(struct _gs831_home_app_ *);
    int (*exit)(struct _gs831_home_app_ *);
    void *userdata;
} gs831_home_app;

typedef void (*_gs831_home_app_func_)(gs831_home_app *);

typedef gs831_home_app (*_get_gs831_home_app_func_)();

// #define ai2vi(val) (int)((val) * (240.0 / 224.0)) // ai / vi = 224 / 240
// #define vi2ai(val) (int)((val) * (224.0 / 240.0)) // vi / ai = 240 / 224

#define ai2vi(val) (int)((val) * 15 / 14) // ai / vi = 224 / 240
#define vi2ai(val) (int)((val) * 14 / 15) // vi / ai = 240 / 224

#define mv2cvL(l) (int(((l)*255) / 100))
#define mv2cvA(a) ((a) + 128)
#define mv2cvB(b) ((b) + 128)

extern "C"
{
    int gs831_protocol_send(uint8_t * data, int len);
    void gs831_load_json_conf(const std::string &path, json5pp::value &cfg, json5pp::value old);
    void gs831_save_json_conf(const std::string &path, json5pp::value &cfg);
}

#endif /*_gs831_uv_*/
