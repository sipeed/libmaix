
#ifndef __DECODER_POSE
#define __DECODER_POSE

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "libmaix_err.h"
#include "libmaix_nn_decoder.h"
typedef struct
{
    int cx;
    int cy;
    float * keypoints;
}libmaix_nn_decoder_pose_result_t;

typedef struct
{
    int num_joints ;
    int image_size;
    float hm_th;
}libmaix_nn_decoder_pose_config_t;

typedef struct
{
    libmaix_nn_decoder_pose_result_t * result;
    libmaix_nn_decoder_pose_config_t* config;
    float * cente_weight;
    int * range_weight_x;
    int * range_weight_y;
    int * repeat_reg_x;
    int * repeat_reg_y;
    int * tmp_reg_x;
    int * tmp_reg_y;
    float * tmp_reg;
    float * res_x ;
    float * res_y;

}pose_param_t;


/********* direct API **********/
extern void max_point(float * centers , float * center_weight ,int feature_map_size , libmaix_nn_decoder_pose_result_t* result );
extern void max_point_without_weight(float * centers , int feature_map_size , libmaix_nn_decoder_pose_result_t* result );

extern libmaix_err_t  pose_decode(libmaix_nn_decoder_pose_config_t * config , libmaix_nn_decoder_pose_result_t * result);

/********* libmaix API ********/

libmaix_err_t libmaix_nn_decoder_pose_init(libmaix_nn_decoder_t * obj, void* config);
libmaix_err_t libmaix_nn_decoder_pose_deinit(libmaix_nn_decoder_t * obj);
libmaix_err_t libmaix_nn_decoder_pose_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t* feature_map, void* result);

libmaix_nn_decoder_t *libmaix_nn_decoder_pose_create();
void libmaix_nn_decoder_pose_destroy(libmaix_nn_decoder_t** obj);

#ifdef __cplusplus
}
#endif

#endif
