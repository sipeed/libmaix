/**
 * maix neural network lib
 * 
 * @copyright © 2020-2021 Sipeed Ltd, All rights reserved
 * @author neucrack
 * @update --2020.12.28--neucrack: create lib
 *         --
 */
#ifndef __LIBMAIX_NN_H__
#define __LIBMAIX_NN_H__

#include "libmaix_err.h"
#include "libmaix_debug.h"
#include "stdint.h"
#include "stdbool.h"
#include "standard_api.h"
#ifdef __cplusplus
extern "C" {
#endif

int Debug_s;


typedef enum
{
    LIBMAIX_NN_DTYPE_UINT8  = 0,
    LIBMAIX_NN_DTYPE_INT8   = 1,
    LIBMAIX_NN_DTYPE_FLOAT  = 2,
}libmaix_nn_dtype_t;               // data type

typedef enum
{
    LIBMAIX_NN_LAYOUT_HWC    = 0,
    LIBMAIX_NN_LAYOUT_CHW    = 1,
}libmaix_nn_layout_t;


typedef union
{
    struct
    {
        char* param_path;
        char* bin_path;
    }awnn;
    struct
    {
        char* model_path;
        char* reserved;
    }normal;
}libmaix_nn_model_path_t;

typedef struct
{
    int w;
    int h;
    int c;
    libmaix_nn_dtype_t dtype;    // for awnn model, should be int8 if not need_quantization
                                 //                 or uint8 when need_quantization
    libmaix_nn_layout_t layout;
    bool  need_quantization;     // need quantize data, for input layer
    void* data;                  // layer data
    void* buff_quantization;     // buffer for quantization temporary usage,
                                 // if need_quantization is true and this buff set, will use this buff,
                                 // if need_quantization but this buff is NULL, will return LIBMAIX_ERR_PARAM error
}libmaix_nn_layer_t;

typedef union
{
    struct
    {
        char**   input_names;
        char**   output_names;
        uint8_t  input_num;               // len(input_names)
        uint8_t  output_num;              // len(output_names)
        float    mean[3];
        float    norm[3];
        int*     input_ids;
        int*     output_ids;
        bool encrypt;
    }awnn;
        struct
    {
        char**   input_names;
        char**   output_names;
        uint8_t  input_num;               // len(input_names)
        uint8_t  output_num;              // len(output_names)
        float    mean[3];
        float    norm[3];
        int*     input_ids;
        int*     output_ids;
        bool encrypt;
    }normal;
}libmaix_nn_opt_param_t; // optional param for model


typedef struct libmaix_nn
{
    void* _config;
    libmaix_err_t (*init)(struct libmaix_nn *obj);
    libmaix_err_t (*deinit)(struct libmaix_nn *obj);
    libmaix_err_t (*load)(struct libmaix_nn *obj, const libmaix_nn_model_path_t* path, libmaix_nn_opt_param_t* opt_param);
    /**
     * @param [in] inputs: dtype and data may be changed by forward
     *                      for awnn, it need to quantize input to int8 if dtype is uint8,
     *                                if dtype is int8, will do nothing, other dtype will return LIBMAIX_ERR_PARAM error
     * @param [out] outputs: output feature maps, outputs[i].data must NOT be NULL, dtype is LIBMAIX_NN_DTYPE_FLOAT
     */
    libmaix_err_t (*forward)(struct libmaix_nn *obj, libmaix_nn_layer_t* inputs, libmaix_nn_layer_t* outputs);
}libmaix_nn_t;

libmaix_err_t libmaix_nn_module_init();
libmaix_err_t libmaix_nn_module_deinit();
libmaix_nn_t* libmaix_nn_create();
void libmaix_nn_destroy(libmaix_nn_t** obj);

float libmaix_nn_feature_compare_int8(int8_t* a, int8_t* b, int len);
float libmaix_nn_feature_compare_float(float* a, float* b, int len);

#ifdef __cplusplus
}
#endif


#endif

