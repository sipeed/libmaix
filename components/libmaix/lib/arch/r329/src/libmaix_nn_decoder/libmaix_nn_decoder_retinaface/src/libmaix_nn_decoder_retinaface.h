/*
    retinaface decoder
    @author neucrack@sipeed
    @date 2021-5-15
          2021-8-18  update for libmaix
    @license MIT
*/

#ifndef __DECODER_RETINAFACE_H
#define __DECODER_RETINAFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "libmaix_err.h"
#include "libmaix_nn_decoder.h"

#define ANCHOR_SIZE_NUM 3

typedef struct
{
    float x;
    float y;
    float w;
    float h;
}retinaface_box_t;

typedef struct
{
    retinaface_box_t box;
    float score;
    float points[10];

    /* internal use*/
    int idx;
}retinaface_face_t;

typedef struct
{
    float variance[2];
    int steps[ANCHOR_SIZE_NUM];
    int min_sizes[ANCHOR_SIZE_NUM * 2];

    float nms;
    float score_thresh;
    int   input_w;
    int   input_h;

    // set by init func
    int   channel_num;
}libmaix_nn_decoder_retinaface_config_t;

typedef struct
{
    retinaface_face_t* faces;
    int                num;
}libmaix_nn_decoder_retinaface_result_t;

/************ direct API ***********/
extern retinaface_box_t* retinaface_get_priorboxes(libmaix_nn_decoder_retinaface_config_t* config, int* boxes_num);
extern libmaix_err_t retinaface_decode(float* net_out_loc, float* net_out_conf, float* net_out_landmark, retinaface_box_t* prior_boxes, retinaface_face_t* faces, int* boxes_num, bool chw, libmaix_nn_decoder_retinaface_config_t* config);
extern int retinaface_get_channel_num(libmaix_nn_decoder_retinaface_config_t* config);

/************ libmaix API **********/
libmaix_err_t libmaix_nn_decoder_retinaface_init(struct libmaix_nn_decoder* obj, void* config);
libmaix_err_t libmaix_nn_decoder_retinaface_deinit(struct libmaix_nn_decoder* obj);
/**
 * 
 * @param[out] result: address of libmaix_nn_decoder_retinaface_result_t variable, faces just set to NULL, and after call, no need to free faces
 */
libmaix_err_t libmaix_nn_decoder_retinaface_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t* feature_map, void* result);

libmaix_nn_decoder_t* libmaix_nn_decoder_retinaface_create();
void libmaix_nn_decoder_retinaface_destroy(libmaix_nn_decoder_t** obj);

#ifdef __cplusplus
}
#endif

#endif

