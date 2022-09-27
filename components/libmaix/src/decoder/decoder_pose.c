#ifdef __cplusplus
extern "C"
{
#endif

#include "libmaix_nn_decoder_pose.h"
#include "libmaix_debug.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
extern void max_point_without_weight(float * centers , int feature_map_size , libmaix_nn_decoder_pose_result_t* result )
{
    float max = -DBL_MAX;
    int max_idx = 0;
    for(int i=0 ; i < feature_map_size * feature_map_size ;i++)
    {
        float tmp = centers[i];
        if (tmp > max)
        {
            max = tmp;
            max_idx = i;
        }
    }
    result->cx = max_idx % feature_map_size;
    result->cy = max_idx / feature_map_size;
}

void max_point(float * centers , float * center_weight,int feature_map_size , libmaix_nn_decoder_pose_result_t* result )
{
    // center weight load by exter API
    float max = -DBL_MAX;
    int max_idx = 0;
    for(int i=0 ; i < feature_map_size * feature_map_size ;i++)
    {
        float tmp = center_weight[i] * centers[i];
        if (tmp > max)
        {
            max = tmp;
            max_idx = i;
        }
    }
    result->cx = max_idx % feature_map_size;
    result->cy = max_idx / feature_map_size;
}

libmaix_nn_decoder_t *libmaix_nn_decoder_pose_create()
{
    libmaix_nn_decoder_t *obj = (libmaix_nn_decoder_t *)malloc(sizeof(libmaix_nn_decoder_t));
    if (!obj)
        return NULL;
    pose_param_t *params = (pose_param_t *)malloc(sizeof(pose_param_t));
    if (!params)
        return NULL;
    memset(obj, 0, sizeof(libmaix_nn_decoder_t));
    memset(params, 0, sizeof(pose_param_t));
    obj->init = libmaix_nn_decoder_pose_init;
    obj->deinit = libmaix_nn_decoder_pose_deinit;
    obj->decode = libmaix_nn_decoder_pose_decode;
    obj->data = params;
    return obj;
}

void libmaix_nn_decoder_pose_destroy(libmaix_nn_decoder_t **obj)
{
    if (*obj)
    {
        if ((*obj)->data)
        {
            free((*obj)->data);
            (*obj)->data = NULL;
        }
        free(*obj);
    }
    *obj = NULL;
}

libmaix_err_t libmaix_nn_decoder_pose_init(libmaix_nn_decoder_t *obj, void *config)
{
    if(! config)
        return LIBMAIX_ERR_PARAM;
    pose_param_t * params = (pose_param_t *)obj->data;
    params->config = (libmaix_nn_decoder_pose_config_t *) config;
    params->result  = (libmaix_nn_decoder_pose_result_t *) malloc(sizeof(libmaix_nn_decoder_pose_result_t));
    if (! params->result)
        return LIBMAIX_ERR_NO_MEM;
    params->result->keypoints = (int *) malloc(sizeof(int) * params->config->num_joints * 2);
    int feature_map_size = params->config->image_size / 4;
    int feature_map_area = feature_map_size * feature_map_size;

    //init range weight
    params->range_weight_x = (int *)malloc(sizeof(int) * feature_map_area);
    params->range_weight_y = (int *)malloc(sizeof(int) * feature_map_area);

    params-> repeat_reg_x = (int *)malloc(sizeof(int) *  feature_map_area);
    params-> repeat_reg_y = (int *)malloc(sizeof(int) *  feature_map_area);
    params-> tmp_reg_x = (int *)malloc(sizeof(int) *  feature_map_area);
    params-> tmp_reg_y = (int *)malloc(sizeof(int) *  feature_map_area);
    params->tmp_reg = (float *)malloc(sizeof(float * ) * feature_map_area);



    for(int i = 0 ; i < feature_map_area ; i++)
    {
        params->range_weight_x[i] =  i % feature_map_size;
    }
    for(int i=0 ; i <feature_map_area ; i ++)
    {
        params->range_weight_y[i] = i / feature_map_size;
    }
    //init center weight
    params->cente_weight = (float *)malloc(sizeof(float) * feature_map_size );
    //TODO
    // inti it

    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_decoder_pose_deinit(libmaix_nn_decoder_t * obj)
{
    pose_param_t * params = (pose_param_t *) obj->data;
    if(params->result)
    {
        if(params->result->keypoints)
        {
            free(params->result->keypoints);
            params->result->keypoints = NULL;
        }
        if(params->range_weight_x)
        {
            free(params->range_weight_x);
            params->range_weight_x = NULL;
        }
        if(params->range_weight_y)
        {
            free(params->range_weight_y);
            params->range_weight_y = NULL;
        }
        free(params->result);
        params->result = NULL;
    }
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_decoder_pose_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t* feature_map, void* result)
{
    if (! result)
        return LIBMAIX_ERR_PARAM;

    pose_param_t * params = (pose_param_t *) obj->data;
    libmaix_nn_decoder_pose_result_t * result_object = (libmaix_nn_decoder_pose_result_t*) result;
    int num_joints = params->config-> num_joints;
    int image_size = params->config->image_size;
    float hm_th = params->config->hm_th;
    int feature_map_size = image_size / 4;

    // get output buffer
    float * heatmaps = (float*)feature_map[0].data;
    float * centers = (float*)feature_map[1].data;
    float * regs = (float*)feature_map[2].data;
    float * offsets = (float*)feature_map[3].data;

    max_point(centers ,  params->cente_weight  ,feature_map_size ,  result_object);
    int cx = result_object->cx;
    int cy = result_object->cy;
    int feature_map_area = feature_map_size * feature_map_size;

    for(int n=0 ; n < num_joints ; n++)
    {
        int location_x = n * feature_map_area + cy * feature_map_size + cx;
        int location_y = location_x + feature_map_area;

        int reg_x_origin = regs[location_x];
        int reg_y_origin = regs[location_y];

        for(int j=0 ; j < feature_map_area ; j++)
        {
            params->repeat_reg_x[j] = reg_x_origin + cx;
            params->repeat_reg_y[j] = reg_y_origin + cy;
        }

        for(int i=0 ; i < feature_map_area ; i++)
        {
            params->tmp_reg_x[i] =(params->range_weight_x[i] - params->repeat_reg_x[i]) * (params->range_weight_x[i] - params->repeat_reg_x[i]);
            params->tmp_reg_y[i] =(params->range_weight_y[i] - params->repeat_reg_y[i]) * (params->range_weight_y[i] - params->repeat_reg_y[i]);
        }

        for(int i=0 ; i < feature_map_area ; i++)
        {
            float tmp_value =  sqrtf(params->tmp_reg_x[i] + params->tmp_reg_y[i]) + 1.8;
            params->tmp_reg [i] = heatmaps[n * feature_map_area + i ] / tmp_value;
        }
        max_point_without_weight(params->tmp_reg , feature_map_size , result_object);

        if (result_object->cx < 0 )
            result_object->cx = 0;
        if (result_object->cy < 0)
            result_object->cy = 0;

        if (result_object->cx > (feature_map_size -1))
            result_object->cx = (feature_map_size - 1);
        if(result_object->cy > (feature_map_size -1))
            result_object->cy = (feature_map_size - 1);

        float score = heatmaps[n * feature_map_area + result_object->cy * feature_map_size + result_object->cx];


        for(int i=0 ; i < feature_map_area ; i++)
        {
            params->res_x[i] = result_object->cx +  offsets[n * feature_map_area + result_object->cy * feature_map_size + result_object->cx ] / feature_map_size;
            params->res_y[i] = result_object->cy +  offsets[(n+1) * feature_map_area + result_object->cy * feature_map_size + result_object->cx ] / feature_map_size;
            if (params->res_x[i]  < score)
                params->res_x[i] = -1 ;
            if(params->res_y[i] < score)
                params->res_y[i] = -1;
        }

    }



    return LIBMAIX_ERR_NONE;
}




#ifdef __cplusplus
}
#endif
