
#ifndef __DECODER_LICENSE_PLATE_LOCATION
#define __DECODER_LICENSE_PLATE_LOCATION

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
    float x;
    float y;
    float w;
    float h;
}LP_box_t;

typedef struct
{
    LP_box_t box;
    float score;
    float points[8];
    int idx;

}LP_t;


typedef struct
{
    float variance[2];
    int steps[3];
    int min_sizes[6];

    float nms;
    float score_thresh;
    int   input_w;
    int   input_h;

    int   channel_num;
}libmaix_nn_decoder_license_plate_location_config_t;

typedef struct
{
    LP_t*  plates;
    int                num;
}libmaix_nn_decoder_license_plate_location_result_t;

/************ direct API ***********/
extern LP_box_t* license_plate_location_get_priorboxes(libmaix_nn_decoder_license_plate_location_config_t* config, int* boxes_num);
extern libmaix_err_t license_plate_location_decode(float* net_out_loc, float* net_out_conf, float* net_out_landmark, LP_box_t* prior_boxes, LP_t* faces, int* boxes_num, bool chw, libmaix_nn_decoder_license_plate_location_config_t* config);
extern int license_plate_location_get_channel_num(libmaix_nn_decoder_license_plate_location_config_t* config);


/************ libmaix API **********/
libmaix_err_t libmaix_nn_decoder_license_plate_location_init(struct libmaix_nn_decoder* obj, void* config);
libmaix_err_t libmaix_nn_decoder_license_plate_location_deinit(struct libmaix_nn_decoder* obj);
/**
 *
 * @param[out] result: address of libmaix_nn_decoder_retinaface_result_t variable, faces just set to NULL, and after call, no need to free faces
 */
libmaix_err_t libmaix_nn_decoder_license_plate_location_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t* feature_map, void* result);

libmaix_nn_decoder_t* libmaix_nn_decoder_license_plate_location_create();
void libmaix_nn_decoder_license_plate_location_destroy(libmaix_nn_decoder_t** obj);

#ifdef __cplusplus
}
#endif

#endif

