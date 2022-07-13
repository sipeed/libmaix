#ifdef __cplusplus
extern "C" {
#endif

#include "libmaix_nn_decoder_ctc.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libmaix_debug.h"

libmaix_nn_decoder_t * libmaix_nn_decoder_ctc_create()
{
    libmaix_nn_decoder_t* obj = (libmaix_nn_decoder_t*)malloc(sizeof(libmaix_nn_decoder_t));
    if(!obj)
        return NULL;
    param_t * params = (param_t*)malloc(sizeof(param_t));
    if (!params)
        return NULL;
    memset(obj, 0, sizeof(libmaix_nn_decoder_t));
    memset(params , 0 , sizeof(param_t));
    obj->init = libmaix_nn_decoder_ctc_init;
    obj->deinit = libmaix_nn_decoder_ctc_deinit;
    obj->decode = libmaix_nn_decoder_ctc_decode;
    obj->data = params;
    return obj;
}

void libmaix_nn_decoder_destroy(libmaix_nn_decoder_t** obj)
{
    if(*obj)
    {
        if((*obj)->data)
        {
            free((*obj)->data);
            (*obj)->data = NULL;
        }
        free(*obj);
    }
    *obj = NULL;
}

libmaix_err_t libmaix_nn_decoder_ctc_init(struct libmaix_nn_decoder* obj, void* config)
{
    if(! config)
    {
        return LIBMAIX_ERR_PARAM;
    }
    param_t * params = (param_t *)obj->data;
    params->config = (libmaix_nn_decoder_ctc_config_t *) config;
    params->result = (libmaix_nn_decoder_ctc_result_t *) malloc (sizeof(libmaix_nn_decoder_ctc_result_t));
    if(! params->result)
    {
        return LIBMAIX_ERR_NO_MEM;
    }
    params->result->label_idxs = (int *)malloc(sizeof(int) * params->config->T);
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_decoder_ctc_deinit(struct libmaix_nn_decoder* obj)
{
    param_t * params = (param_t*)obj->data;
    if(params->result)
    {
        if(params->result->converted_string)
        {
            free(params->result->converted_string);
            params->result->converted_string = NULL;
        }
        if(params->result->label_idxs)
        {
            free(params->result->label_idxs);
            params->result->label_idxs = NULL;
        }
        free(params->result);
        params->result = NULL;
    }
    return LIBMAIX_ERR_NONE;
}


libmaix_err_t libmaix_nn_decoder_ctc_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t *feature_map ,void* result)
{
    // feature must be NCT , for example license plate (1, 68 ,18)
    if(!result)
    {
        return LIBMAIX_ERR_PARAM;
    }
    libmaix_nn_decoder_ctc_result_t* result_obj = (libmaix_nn_decoder_ctc_result_t*)result;
    param_t* params = (param_t*)obj->data;
    int T = params->config->T;
    int C = params->config->C;
    float *buffer = feature_map->data;

    // get max value of each T;
    for(int t=0 ; t != T ; t++)
    {
        float temp_max = buffer[t];
        params->result->label_idxs[t] = 0 ;

        for(int c=0 ;  c != C ; C++)
        {
            if(buffer[t + T*c] > temp_max)
            {
                temp_max = buffer[t + T*c];
                params->result->label_idxs[t] = c;
            }
        }
    }


    //dropout repeate label and blank label
    int char_count = 0 ;
    int blank_idx = params->config->classes_num - 1;
    int pre_c = params->result->label_idxs[0];

    if(pre_c != blank_idx)
    {
        params->result->no_repeat_idx[char_count] = pre_c;
        char_count ++;
    }

    for(int t=0 ; t<T ; t++)
    {
        int c = params->result->label_idxs[t];
        if((pre_c == c) || (c == blank_idx))
        {
            if(c == blank_idx)
            {
                pre_c = c;
            }
            continue;
        }
         params->result->no_repeat_idx[char_count] = c;
         char_count ++;
         pre_c = c;
    }
    params->result->length = char_count;

    //convert to string
    params->result->converted_string = (char *)malloc(sizeof(char) * params->result->length);
    if (! params->result->converted_string)
    {
        return LIBMAIX_ERR_NO_MEM;
    }
    for(int idx=0 ; idx < params->result->length ; idx++)
    {
        int id  = params->result->no_repeat_idx[idx];
        params->result->converted_string[idx] = params->config->labels[id];
    }
    strcpy((char *)result ,params->result->converted_string);
    return LIBMAIX_ERR_NONE ;
}

#ifdef __cplusplus
}
#endif