#ifndef __LIBMAIX_NN_FACE_H__
#define __LIBMAIX_NN_FACE_H__

#include "libmaix_err.h"
#include "libmaix_debug.h"
#include "libmaix_nn.h"
#include "libmaix_image.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float detect_nms;
    float detect_thresh;
    int   detect_input_w;
    int   detect_input_h;
} libmaix_nn_face_config_t;

typedef struct
{
    struct
    {
        int x;
        int y;
    } point[5];
} key_point_t;

typedef struct
{
    int     x1;
    int     y1;
    int     x2;
    int     y2;
    key_point_t key_point;

    float        prob;        // face prob
    float *    feature;

    libmaix_image_t std_img;
} face_obj_t;

typedef void (*libmaix_nn_face_small2big_func_t)(const int* in_x, const int* in_y, int* out_x, int* out_y, void* args);

/**
 * @max_face_num max face result number, now only support 1, so fixed to 1
 * @map_func reserved, just pass NULL. map small image's point to big image point
 */
libmaix_err_t libmaix_nn_face_recognize_init(void** obj, libmaix_nn_face_config_t* config, libmaix_nn_t* model_detect, libmaix_nn_t* model_landmark, libmaix_nn_t* model_feature, int fea_len, int max_face_num, libmaix_nn_face_small2big_func_t map_func, void* map_func_args);
libmaix_err_t libmaix_nn_face_recognize_deinit(void** obj);
/**
 * 
 * @face_num[out] faces number found
 * @img_big reserved
 * @results address of pointer, value of pointer should be NULL
 */
libmaix_err_t libmaix_nn_face_get_feature(void* obj, libmaix_image_t* img, int* face_num, libmaix_image_t* img_big, face_obj_t** results, bool get_std_img);

#ifdef __cplusplus
}
#endif


#endif

