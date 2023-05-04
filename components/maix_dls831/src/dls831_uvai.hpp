
#ifndef _dls831_uv_
#define _dls831_uv_

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
#include "linux_gpio.h"

#include "gpio_lib.hpp"

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
// #include "h264encode.hpp"

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

static void msleep(unsigned int ms)
{
    usleep(ms * 1000);
    // struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
    // nanosleep(&ts, nullptr);
}

#define USE_UI 1

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

 // 160x90:16x9 240x180:4x3
const int dls831_ui_w = 240, dls831_ui_h = 180;
const int dls831_vi_w = 240, dls831_vi_h = 180;
const int dls831_ai_w = 240, dls831_ai_h = 180;

struct dls831_pack_t
{
    uint8_t type;
    std::vector<uint8_t> data;
};

typedef struct
{
    // sys
    const char *config_file = "/tmp/dls831_home.json";
    json5pp::value config_json;
    const char *config_file_color = "/root/color_config.json";
    json5pp::value config_json_color;
    uint8_t update_color_state = 0;
    uint8_t current_app_id,new_app_id;
    int exit, signal;
    pthread_mutex_t vi_mutex, ai_mutex, ui_mutex;
    // hw
    struct timeval timeout;
    uint32_t input_event0, dev_ttyS0, dev_ttyS1, sensor_flip, sensor_time, toferr_time, toferr_sum;
    fd_set readfd;
    std::list<dls831_pack_t> recvPacks;
    // vi
    uint16_t vi_w, vi_h, ai_w, ai_h, ui_w, ui_h;
    libmaix_cam *vi, *ai;
    uint8_t *vi_yuv;
    // uint8_t *ai_rgb;
    // libmaix_image_t *ai_rgb;
    libmaix_vo *vo;
    // cv::Mat ui_bgra;
    // libmaix_image_t *ui_rgba;
    // ui
    pthread_t ui_thread;
    int ui_th_ms, vi_th_ms;
    // ai
    pthread_t ai_thread;
    int ai_th_ms, ai_th_id;
    volatile int ai_th_keep;
    time_t start;
    uint32_t old;
} dls831_uv;

typedef struct _dls831_home_app_
{
    int (*load)(struct _dls831_home_app_ *);
    int (*loop)(struct _dls831_home_app_ *);
    int (*exit)(struct _dls831_home_app_ *);
    void *userdata;
} dls831_home_app;

typedef void (*_dls831_home_app_func_)(dls831_home_app *);

typedef dls831_home_app (*_get_dls831_home_app_func_)();

#define ai2vi(val) (int)((val) * (240.0 / 224.0)) // ai / vi = 224 / 240
#define vi2ai(val) (int)((val) * (224.0 / 240.0)) // vi / ai = 240 / 224

#define ai2vi_w(val) (int)((val) * dls831_vi_w / dls831_ai_w)
#define ai2vi_h(val) (int)((val) * dls831_vi_h / dls831_ai_h)
#define vi2ai_w(val) (int)((val) * dls831_ai_w / dls831_vi_w)
#define vi2ai_h(val) (int)((val) * dls831_ai_h / dls831_ai_h)

#define mv2cvL(l) (int(((l)*255) / 100))
#define mv2cvA(a) ((a) + 128)
#define mv2cvB(b) ((b) + 128)

extern "C"
{
    void dls831_write_string_to_file(std::string path, std::string txt);
	void dls831_save_json_conf(const std::string &path, json5pp::value &cfg);
    void dls831_load_json_conf(const std::string &path, json5pp::value &cfg, json5pp::value old);
    int dls831_home_app_select(int id);
    void dls831_home_app_stop();
    uint32_t dls831_get_ms();
    int dls831_protocol_send(uint8_t * data, int len);

    int AW_MPI_VI_SetVippMirror(int ViDev, int Value);
    int AW_MPI_VI_SetVippFlip(int ViDev, int Value);
    // int AW_MPI_ISP_AE_SetMode(int IspDev, int Value);			// [0:auto, 1:manual]
    // int AW_MPI_ISP_AE_SetExposure(int IspDev, int Value);		// [0, 65535*16]
    // int AW_MPI_ISP_AE_SetGain(int IspDev, int Value);			// [0, 65535]

    static int set_exp_gain(int exp, int gain)
    {
        // if (exp == 0 && gain == 0) {
        //     AW_MPI_ISP_AE_SetMode(0, 0);
        // }
        // else if(exp == 0)
        // {
        //     AW_MPI_ISP_AE_SetMode(0, 1);
        //     AW_MPI_ISP_AE_SetGain(0, gain);
        //     // AW_MPI_ISP_AE_SetExposure(0, exp);
        // }
        // else if (gain == 0)
        // {
        //     AW_MPI_ISP_AE_SetMode(0, 1);
        //     // AW_MPI_ISP_AE_SetGain(0, gain);
        //     AW_MPI_ISP_AE_SetExposure(0, exp);
        // }
        // else
        // {
        //     AW_MPI_ISP_AE_SetMode(0, 1);
        //     AW_MPI_ISP_AE_SetGain(0, gain);
        //     AW_MPI_ISP_AE_SetExposure(0, exp);
        // }
        return 0;
    }

}

#endif /*_dls831_uv_*/
