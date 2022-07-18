#ifndef __LIBMAIX_NN_DECODER_CTC_H
#define __LIBMAIX_NN_DECODER_CTC_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "libmaix_err.h"
#include "libmaix_nn_decoder.h"

#define debug_line  //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)

//result
typedef struct
{
    int *label_idxs;
    int *no_repeat_idx;
    char *converted_string;
    int length;
}libmaix_nn_decoder_ctc_result_t;

typedef struct
{
    char **labels;
    int classes_num;
    int T;
    int N;
    int C;
    int lpr_max_lenght;
}libmaix_nn_decoder_ctc_config_t;

typedef struct
{
    libmaix_nn_decoder_ctc_result_t * result;
    libmaix_nn_decoder_ctc_config_t* config;
}param_t;


/************ direct API ***********/
extern libmaix_err_t ctc_decode(float * output , libmaix_nn_decoder_ctc_config_t* config , libmaix_nn_decoder_ctc_result_t * result);

/************ libmaix API **********/
libmaix_err_t libmaix_nn_decoder_ctc_init(struct libmaix_nn_decoder* obj, void* config);
libmaix_err_t libmaix_nn_decoder_ctc_deinit(struct libmaix_nn_decoder* obj);
libmaix_err_t libmaix_nn_decoder_ctc_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t *feature_map ,void* result );
libmaix_nn_decoder_t * libmaix_nn_decoder_ctc_create();
void libmaix_nn_decoder_ctc_destroy(libmaix_nn_decoder_t** obj);

#ifdef __cplusplus
}
#endif
#endif