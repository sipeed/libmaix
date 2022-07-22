

#define debug_line  //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)
// #define debug_line  printf()
#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "libmaix_nn_decoder_license_plate_location.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libmaix_debug.h"

int LP_min_size_len = 6;
int LP_anchor_size_len = 3;


static void softmax(float *data, int stride, int n )
{
    int i;
    float sum = 0;
    float largest_i = data[0];

    for (i = 0; i < n; ++i)
    {
        if (data[i + stride] > largest_i)
            largest_i = data[i + stride];
    }
    for (i = 0; i < n; ++i)
    {
        float value = expf(data[i + stride] - largest_i);
        sum += value;
        data[i + stride] = value;
    }
    for (i = 0; i < n; ++i)
	{
        data[i + stride] /= sum;
	}
}

static float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1 / 2;
    float l2 = x2 - w2 / 2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1 / 2;
    float r2 = x2 + w2 / 2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static float box_intersection(LP_box_t* a, LP_box_t* b)
{
    float w = overlap(a->x, a->w, b->x, b->w);
    float h = overlap(a->y, a->h, b->y, b->h);

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static float box_union(LP_box_t* a, LP_box_t* b)
{
    float i = box_intersection(a, b);
    float u = a->w * a->h + b->w * b->h - i;
    return u;
}

static float box_iou(LP_box_t* a, LP_box_t* b)
{
    return box_intersection(a, b) / box_union(a, b);
}

typedef struct
{
    int index;
    int class_id;
    LP_t* plates;
}sortable_box_t;


static int nms_comparator(const void *pa, const void *pb)
{
    sortable_box_t* a = (sortable_box_t *)pa;
    sortable_box_t* b = (sortable_box_t *)pb;
    float diff = a->plates[a->index].score - b->plates[b->index].score;

    // if (diff < 0)
    //     return 1;
    // else if (diff > 0)
    //     return -1;
    // return 0;
    return (int)(-*(int32_t*)(&diff));
}



static void do_nms_sort(uint32_t boxes_number, float nms_value, float score_thresh, LP_t* plates)
{
    uint32_t i = 0, j = 0, k = 0;
    sortable_box_t s[boxes_number];

    for (i = 0; i < boxes_number; ++i)
    {
        s[i].index = i;
        s[i].class_id = 0;
        s[i].plates = plates;
    }
    // for (k = 0; k < classes; ++k) // only one(face) class
    {
        for (i = 0; i < boxes_number; ++i)
            s[i].class_id = k;

        qsort(s, boxes_number, sizeof(sortable_box_t), nms_comparator);  // conf sort
        for (i = 0; i < boxes_number; ++i)
        {
            if (plates[s[i].index].score < score_thresh)
                continue;
            LP_box_t* a = &plates[s[i].index].box;

            for (j = i + 1; j < boxes_number; ++j)
            {
                LP_box_t* b = &plates[s[j].index].box;

                if (box_iou(a, b) > nms_value)
                    plates[s[j].index].score = 0;
            }
        }
    }
}

int license_plate_location_get_channel_num(libmaix_nn_decoder_license_plate_location_config_t* config)
{

    int anchor_num = 0;
    printf("get\n");
    for(int i=0; i < LP_anchor_size_len; ++i)
    {
        anchor_num += (config->input_w / config->steps[i] )* (config->input_h / config->steps[i])  * 2;
    }

    debug_line;
    return anchor_num;
}

LP_box_t* license_plate_location_get_priorboxes(libmaix_nn_decoder_license_plate_location_config_t* config, int* boxes_num)
{

    int anchors_size[LP_anchor_size_len * 2];
    int anchor_num = 0;
    int count = 0;

    for(int i=0; i < LP_anchor_size_len; ++i)
    {
        anchors_size[i * 2] = ceil(config->input_h * 1.0 / config->steps[i]);
        anchors_size[i * 2 + 1] = ceil(config->input_w * 1.0 / config->steps[i]);
        anchor_num += anchors_size[i * 2] * anchors_size[i * 2 + 1] * 2;
    }

    *boxes_num = anchor_num;

    LP_box_t* boxes = (LP_box_t*)malloc(sizeof(LP_box_t) * anchor_num);
    if(!boxes)
    {
        printf("malloc fail\n");
        return NULL;
    }

    for(int i=0; i < LP_anchor_size_len; ++i)
    {
        for(int j=0; j < anchors_size[i * 2]; ++j)
        {
            for(int k=0; k < anchors_size[i * 2 + 1]; ++k)
            {
                for(int m=0; m < 2; ++m)
                {
                    int min_size = config->min_sizes[i * 2 + m];
                    boxes[count].x = (k + 0.5) * config->steps[i] / config->input_w;
                    boxes[count].y = (j + 0.5) * config->steps[i] / config->input_h;
                    boxes[count].w = min_size * 1.0 / config->input_w;
                    boxes[count].h = min_size * 1.0 / config->input_h;
                    ++count;
                }
            }
        }
    }

    debug_line;

    return boxes;
}

libmaix_err_t license_plate_location_decode(float* net_out_loc, float* net_out_conf, float* net_out_landmark, LP_box_t* prior_boxes, LP_t* plates, int* boxes_num, bool chw, libmaix_nn_decoder_license_plate_location_config_t* config)
{
    int valid_boxes_count = 0;
    int all_boxes_num = *boxes_num;
    int idx = 0;
    debug_line;
    if(!chw) // hwc: [[[x, x,x,x,....], [y,y,y,y..][w....], [h...]]]
    {
        debug_line;
        /* 1 remove boxes which score < threshhold */
        for(int i=0; i < *boxes_num; ++i)
        {
            /* 1.1 softmax */
            softmax(net_out_conf + i, all_boxes_num, 2);

            /* 1.2. decode conf score */
            plates[i].score = net_out_conf[all_boxes_num + i];

            /* 1.3 tag only copy valid plates info*/
            if(plates[i].score > config->score_thresh)
            {
                plates[valid_boxes_count].score = plates[i].score;
                plates[valid_boxes_count].idx = i;
                ++valid_boxes_count;
            }
        }
        *boxes_num = valid_boxes_count;

        for(int i=0; i < *boxes_num; ++i)
        {
            idx = plates[i].idx;

            /* 2. decode boxes*/
            plates[i].box.x = prior_boxes[idx].x + net_out_loc[idx] * config->variance[0] * prior_boxes[idx].w;
            plates[i].box.y = prior_boxes[idx].y + net_out_loc[idx + all_boxes_num] * config->variance[0] * prior_boxes[idx].h;
            plates[i].box.w = prior_boxes[idx].w * exp(net_out_loc[idx + all_boxes_num * 2] * config->variance[1]);
            plates[i].box.h = prior_boxes[idx].h * exp(net_out_loc[idx + all_boxes_num * 3] * config->variance[1]);
            plates[i].box.x = plates[i].box.x - plates[i].box.w / 2.0;
            plates[i].box.y = plates[i].box.y - plates[i].box.h / 2.0;

            /* 3. decode landmarks*/
            plates[i].points[0] = prior_boxes[idx].x + net_out_landmark[idx] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[1] = prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[2] = prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 2] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[3] = prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 3] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[4] = prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 4] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[5] = prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 5] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[6] = prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 6] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[7] = prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 7] * config->variance[0] * prior_boxes[idx].h;
        }
    }
    else    // chw: x,y,w,h......x,y,w,h
    {
        debug_line;
        /* 1 remove boxes which score < threshhold */
        // CALC_TIME_START();
        for(int i=0; i < *boxes_num; i++)
        {
            /* 1.1 softmax */
            // LIBMAIX_DEBUG_PRINTF("%f, %f ==> ", net_out_conf[i * 2 ], net_out_conf[i * 2 + 1]);
            softmax(net_out_conf + i * 2, 0, 2);
            // LIBMAIX_DEBUG_PRINTF("%f, %f\n", net_out_conf[i * 2 ], net_out_conf[i * 2 + 1]);
            /* 1.2. decode conf score */
            plates[i].score = net_out_conf[i * 2 +1 ];

            /* 1.3 tag only copy valid plates info*/
            if(plates[i].score > config->score_thresh)
            {
                plates[valid_boxes_count].score = plates[i].score;
                plates[valid_boxes_count].idx = i;
                ++valid_boxes_count;
            }
        }
        *boxes_num = valid_boxes_count;
        //  debug_line("[libmaix_nn decoder ] valid_boxes_count is %d\n",valid_boxes_count);
        // CALC_TIME_END("find valid boxes");
        // CALC_TIME_START();

        for(int i=0; i < *boxes_num; ++i)
        {
            idx = plates[i].idx;
            /* 2. decode boxes*/
            plates[i].box.x = prior_boxes[idx].x + net_out_loc[idx * 4] * config->variance[0] * prior_boxes[idx].w;
            plates[i].box.y = prior_boxes[idx].y + net_out_loc[idx * 4 + 1] * config->variance[0] * prior_boxes[idx].h;
            plates[i].box.w = prior_boxes[idx].w * exp(net_out_loc[idx * 4 + 2] * config->variance[1]);
            plates[i].box.h = prior_boxes[idx].h * exp(net_out_loc[idx * 4 + 3] * config->variance[1]);
            plates[i].box.x = plates[i].box.x - plates[i].box.w / 2.0; // 左上角
            plates[i].box.y = plates[i].box.y - plates[i].box.h / 2.0;  // 左上角
            // debug_line("%f %f %f %f, %f %f, %f %f\n", plates[i].box.x, plates[i].box.y, plates[i].box.w, plates[i].box.h, prior_boxes[i].w , prior_boxes[i].h, net_out_loc[i * 4 + 2], net_out_loc[i * 4 + 3]);

            /* 3. decode landmarks*/
            plates[i].points[0] = prior_boxes[idx].x + net_out_landmark[idx * 8] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[1] = prior_boxes[idx].y + net_out_landmark[idx * 8 + 1] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[2] = prior_boxes[idx].x + net_out_landmark[idx * 8 + 2] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[3] = prior_boxes[idx].y + net_out_landmark[idx * 8 + 3] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[4] = prior_boxes[idx].x + net_out_landmark[idx * 8 + 4] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[5] = prior_boxes[idx].y + net_out_landmark[idx * 8 + 5] * config->variance[0] * prior_boxes[idx].h;
            plates[i].points[6] = prior_boxes[idx].x + net_out_landmark[idx * 8 + 6] * config->variance[0] * prior_boxes[idx].w;
            plates[i].points[7] = prior_boxes[idx].y + net_out_landmark[idx * 8 + 7] * config->variance[0] * prior_boxes[idx].h;
        }
        // CALC_TIME_END("decode valid boxes");
    }
    /* 4. nms, remove boxes */
    // CALC_TIME_START();
    debug_line;
    do_nms_sort(*boxes_num, config->nms, config->score_thresh, plates);
    debug_line;
    // CALC_TIME_END("do nms");
    return LIBMAIX_ERR_NONE;
}

/**************** libmaix wrapper ********************/
typedef struct
{
    LP_box_t*   priors;
    int                 boxes_num;
    LP_t*  plates;
    libmaix_nn_decoder_license_plate_location_config_t* config;
}obj_params_t;

libmaix_err_t libmaix_nn_decoder_license_plate_location_init(struct libmaix_nn_decoder* obj, void* config)
{
    obj_params_t* params = (obj_params_t*)obj->data;
    params->config = (libmaix_nn_decoder_license_plate_location_config_t*)config;
    params->priors = license_plate_location_get_priorboxes(params->config, &(params->boxes_num));
    if(!params->priors)
    {
        return LIBMAIX_ERR_NO_MEM;
    }
    params->plates = (LP_t*)malloc(sizeof(LP_t) * params->boxes_num);
    if(!params->plates)
    {
        printf("[libmaix decoder ]  allocate plates buffer is faild \n");
        free(params->priors);
        params->priors = NULL;
        return LIBMAIX_ERR_NO_MEM;
    }
    debug_line;
    ((libmaix_nn_decoder_license_plate_location_config_t*)config)->channel_num = license_plate_location_get_channel_num((libmaix_nn_decoder_license_plate_location_config_t*)config);
    debug_line;
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_decoder_license_plate_location_deinit(struct libmaix_nn_decoder* obj)
{
    obj_params_t* params = (obj_params_t*)obj->data;
    if(params->priors)
    {
        free(params->priors);
        params->priors = NULL;
    }
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_decoder_license_plate_location_decode(struct libmaix_nn_decoder* obj, libmaix_nn_layer_t* feature_map, void* result)
{
    if(!result)
    {
        return LIBMAIX_ERR_PARAM;
    }
    libmaix_nn_decoder_license_plate_location_result_t* result_obj = (libmaix_nn_decoder_license_plate_location_result_t*)result;

    obj_params_t* params = (obj_params_t*)obj->data;
    if(!params->priors)
    {
        return LIBMAIX_ERR_NOT_INIT;
    }
    result_obj->plates = params->plates;
    int valid_boxes = params->boxes_num;
    debug_line;
    libmaix_err_t err = license_plate_location_decode((float*)feature_map[0].data, (float*)feature_map[1].data, (float*)feature_map[2].data,
                        params->priors,
                        result_obj->plates, &valid_boxes, feature_map[0].layout == LIBMAIX_NN_LAYOUT_CHW, params->config);

    debug_line;
    result_obj->num = valid_boxes;
    return err;
}


libmaix_nn_decoder_t* libmaix_nn_decoder_license_plate_location_create()
{
    libmaix_nn_decoder_t* obj = (libmaix_nn_decoder_t*)malloc(sizeof(libmaix_nn_decoder_t));

    if(!obj)
        return NULL;
    obj_params_t* params = (obj_params_t*)malloc(sizeof(obj_params_t));
    if(!params)
    {
        free(obj);
        return NULL;
    }
    memset(obj, 0, sizeof(libmaix_nn_decoder_t));
    memset(params, 0, sizeof(obj_params_t));
    obj->init = libmaix_nn_decoder_license_plate_location_init;
    obj->deinit = libmaix_nn_decoder_license_plate_location_deinit;
    obj->decode = libmaix_nn_decoder_license_plate_location_decode;
    obj->data = params;
    return obj;
}

void libmaix_nn_decoder_license_plate_location_destroy(libmaix_nn_decoder_t** obj)
{
    if(*obj)
    {
        if((*obj)->data)
        {
            obj_params_t* params = (obj_params_t*)(*obj)->data;
            if(params->priors)
            {
                free(params->priors);
            }
            free((*obj)->data);
        }
        free(*obj);
    }
    *obj = NULL;
}




#ifdef __cplusplus
}
#endif