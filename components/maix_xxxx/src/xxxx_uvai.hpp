
#ifndef _xxxx_uv_
#define _xxxx_uv_

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

const int xxxx_vi_w = 240, xxxx_vi_h = 240;
const int xxxx_ui_w = 240, xxxx_ui_h = 240;
const int xxxx_ai_w = 224, xxxx_ai_h = 224;

typedef struct
{
    int exit, signal;
    pthread_mutex_t vi_mutex, ai_mutex;
    // hw
    struct timeval timeout;
    int input_event0, dev_ttyS1;
    fd_set readfd;
    // vi
    uint16_t vi_w, vi_h, ai_w, ai_h, ui_w, ui_h;
    libmaix_cam *vi, *ai;
    libmaix_image_t *ai_rgb;
    libmaix_vo *vo;
} xxxx_uv;

// #define ai2vi(val) (int)((val) * (240.0 / 224.0)) // ai / vi = 224 / 240
// #define vi2ai(val) (int)((val) * (224.0 / 240.0)) // vi / ai = 240 / 224

#define ai2vi(val) (int)((val) * 15 / 14) // ai / vi = 224 / 240
#define vi2ai(val) (int)((val) * 14 / 15) // vi / ai = 240 / 224

#define mv2cvL(l) (int(((l)*255) / 100))
#define mv2cvA(a) ((a) + 128)
#define mv2cvB(b) ((b) + 128)

extern "C"
{
    void xxxx_vi_show(cv::Mat cv_src);
}

#endif /*_xxxx_uv_*/
