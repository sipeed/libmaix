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
#include <unistd.h>

    extern int get_bin_size(char *filename)
    {
        int size = 0;
        FILE *fp = fopen(filename , "rb");
        if(fp)
        {
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fclose(fp);
        }
        printf("\nfilename=%s,size=%d \n",filename,size);
        return size;
    }

    extern libmaix_err_t read_bin(char *path, float *buf, int size)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        FILE * fp ;
        if((fp = fopen(path , "rb")) == NULL)
        {
            LIBMAIX_DEBUG_PRINTF("\nCan not open the path: %s \n", path);
            err = LIBMAIX_ERR_NOT_EXEC;
            return err;
        }
        LIBMAIX_DEBUG_PRINTF();
        fread(buf , sizeof(float) , size/4  , fp);
        fclose(fp);


        return err;
    }

    int float_to_int(float f)
    {
        int *p = (int *)&f;
        int temp = *p;
        int sign = -1; //判断符号位
        if ((temp & 0x80000000) == 0)
        {
            sign = 1;
        }
        int exp = ((temp >> 23) & 0x000000ff) - 127; //求出指数位
        int tail = (temp & 0x007fffff) | 0x00800000; //求出尾数位
        int res = tail >> (23 - exp);                //求出有效数字
        return sign * res;                           //返回整数
    }

    extern void max_point_without_weight(float *centers, int feature_map_size, int joint_idx, int *joint_x, int *joint_y, libmaix_nn_decoder_pose_result_t *result)
    {
        float max = -100000;
        int max_idx = 0;
        int _feature_map_area = feature_map_size * feature_map_size;
        for (int i = 0; i < _feature_map_area; i++)
        {
            float tmp = (float)centers[i];
            if (tmp > max)
            {

                max = tmp;
                max_idx = i;
            }
        }
        // result->keypoints[ 2 * joint_idx] = max_idx % feature_map_size;
        // result->keypoints[ 2 * joint_idx + 1] = max_idx / feature_map_size;
        *joint_x = max_idx % feature_map_size;
        *joint_y = max_idx / feature_map_size;
    }

    extern void max_point(float *centers, float *center_weight, int feature_map_size, libmaix_nn_decoder_pose_result_t *result)
    {
        // center weight load by exter API
        float max = -100000;
        LIBMAIX_DEBUG_PRINTF();
        int max_idx = 0;
        int _feature_map_area = feature_map_size * feature_map_size;
        // printf("feature map area:%d\n",_feature_map_area);
        for (int i=0; i < _feature_map_area; i++)
        {
            // printf("heatmap %f \n",*(centers + i));
            // printf("weight %f \n",*(center_weight + i));
            float tmp = (float) center_weight[i] * centers[i];

            if (tmp > max)
            {
                max = tmp;
                max_idx = i;
            }
        }
        result->cx = max_idx % feature_map_size;
        result->cy = max_idx / feature_map_size;
        // printf("(x:%d ,y:%d)\n",result->cx ,result->cy);
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
        LIBMAIX_DEBUG_PRINTF();
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        if (!config)
        {
            err = LIBMAIX_ERR_PARAM;
            return err;
        }
        pose_param_t *params = (pose_param_t *)obj->data;
        params->config = (libmaix_nn_decoder_pose_config_t *)config;
        params->result = (libmaix_nn_decoder_pose_result_t *)malloc(sizeof(libmaix_nn_decoder_pose_result_t));
        if (!params->result)
        {
            err = LIBMAIX_ERR_NO_MEM;
            return err;
        }
        params->result->keypoints = (int *)malloc(sizeof(int) * params->config->num_joints * 2);
        LIBMAIX_DEBUG_PRINTF();
        if(! params->result->keypoints)
        {
            err = LIBMAIX_ERR_NO_MEM;
            return err;
        }
        memset(params->result->keypoints , 0 , 4 * params->config->num_joints *2);

        LIBMAIX_DEBUG_PRINTF();
        int feature_map_size = params->config->image_size / 4;
        int feature_map_area = feature_map_size * feature_map_size;
        LIBMAIX_DEBUG_PRINTF();

        params->range_weight_x = (int *)malloc(sizeof(int) * feature_map_area);
        if(params->range_weight_x == NULL)
        {
            err = LIBMAIX_ERR_NO_MEM;
            return err;
        }
        params->range_weight_y = (int *)malloc(sizeof(int) * feature_map_area);
        if(params->range_weight_y == NULL)
        {
            err = LIBMAIX_ERR_NO_MEM;
            return err;
        }
        for (int i = 0; i < feature_map_area; i++)
        {
            params->range_weight_x[i] = i % feature_map_size;
            params->range_weight_y[i] = i / feature_map_size;
        }


        params->cente_weight = (float *)malloc(sizeof(float) * feature_map_area);
        if(params->cente_weight == NULL)
        {
            err = LIBMAIX_ERR_NO_MEM;
            return err;
        }
        if (0 != access(params->config->center_weight_path,F_OK))
        {
            err = LIBMAIX_ERR_NOT_EXEC;
            LIBMAIX_ERROR_PRINTF("open center weight file fail: %s\n", libmaix_get_err_msg(err));
            return err;
        }
        LIBMAIX_DEBUG_PRINTF();
        int center_size = get_bin_size(params->config->center_weight_path);
        if (center_size == 0)
        {
            err = LIBMAIX_ERR_NOT_EXEC;
            LIBMAIX_ERROR_PRINTF("get center weight bin size fail: %s\n", libmaix_get_err_msg(err));
            return err;
        }


        err = read_bin(params->config->center_weight_path , params->cente_weight , center_size);
        printf("center size is %d \n", center_size);

        LIBMAIX_DEBUG_PRINTF();
        return err;
    }

    libmaix_err_t libmaix_nn_decoder_pose_deinit(libmaix_nn_decoder_t *obj)
    {
        pose_param_t *params = (pose_param_t *)obj->data;
        if (params->result)
        {
            if (params->result->keypoints)
            {
                free(params->result->keypoints);
                params->result->keypoints = NULL;
            }
            if (params->range_weight_x)
            {
                free(params->range_weight_x);
                params->range_weight_x = NULL;
            }
            if (params->range_weight_y)
            {
                free(params->range_weight_y);
                params->range_weight_y = NULL;
            }
            free(params->result);
            params->result = NULL;
        }
        return LIBMAIX_ERR_NONE;
    }

    libmaix_err_t libmaix_nn_decoder_pose_decode(struct libmaix_nn_decoder *obj, libmaix_nn_layer_t *feature_map, void *result)
    {
        LIBMAIX_DEBUG_PRINTF();
        if (!result)
            return LIBMAIX_ERR_PARAM;
        LIBMAIX_DEBUG_PRINTF();
        pose_param_t *params = (pose_param_t *)obj->data;
        libmaix_nn_decoder_pose_result_t *outer_result_object = (libmaix_nn_decoder_pose_result_t *)result;

        int num_joints = params->config->num_joints;
        int image_size = params->config->image_size;
        float hm_th = params->config->hm_th;
        int feature_map_size = image_size / 4;

        // get output buffer
        LIBMAIX_DEBUG_PRINTF();
        float *heatmaps = (float *)feature_map[0].data;
        float *centers = (float *)feature_map[1].data;
        float *regs = (float *)feature_map[2].data;
        float *offsets = (float *)feature_map[3].data;
        LIBMAIX_DEBUG_PRINTF();
        memset(params->result->keypoints , 0 , 4 * params->config->num_joints *2);
        max_point((float *)feature_map[1].data, (float*)params->cente_weight, feature_map_size, params->result);
        LIBMAIX_DEBUG_PRINTF();
        int cx = params->result->cx;
        int cy = params->result->cy;
        int feature_map_area = feature_map_size * feature_map_size;

        LIBMAIX_DEBUG_PRINTF();
        for (int n = 0; n < num_joints; n++)
        {
            int location_x = n * feature_map_area + cy * feature_map_size + cx;
            int location_y = location_x + feature_map_area;

            float reg_x_origin = regs[location_x] + 0.5;
            float reg_y_origin = regs[location_y] + 0.5;
            LIBMAIX_DEBUG_PRINTF();
            int reg_x = float_to_int(reg_x_origin) + cx;
            int reg_y = float_to_int(reg_y_origin) + cy;
            LIBMAIX_DEBUG_PRINTF();
            float regs_result[feature_map_area];
            for (int i = 0; i < feature_map_area; i++)
            {
                float tmp_reg_x = (params->range_weight_x[i] - reg_x) * (params->range_weight_x[i] - reg_x);
                float tmp_reg_y = (params->range_weight_y[i] - reg_y) * (params->range_weight_y[i] - reg_y);
                float tmp_reg = sqrtf(tmp_reg_x + tmp_reg_y) + 1.8;
                regs_result[i] = heatmaps[n * feature_map_area + i] / tmp_reg;
            }

            int joint_x;
            int joint_y;
            LIBMAIX_DEBUG_PRINTF();
            max_point_without_weight(regs_result, feature_map_size, n, &joint_x, &joint_y, params->result);
            LIBMAIX_DEBUG_PRINTF();

            //  clip the position of joint
            if (joint_x > feature_map_size - 1)
                joint_x = feature_map_size - 1;
            if (joint_x < 0)
                joint_x = 0;

            if (joint_y > feature_map_size - 1)
                joint_y = feature_map_size - 1;
            if (joint_y < 0)
                joint_y = 0;

            LIBMAIX_DEBUG_PRINTF();
            float score = heatmaps[n * feature_map_area + params->result->cy * feature_map_size + params->result->cx];
            LIBMAIX_DEBUG_PRINTF();
            // int joint_x = result_object->keypoints[2 * n];
            // int joint_y = result_object->keypoints[2 * n+1];
            int offset_location_x = n * feature_map_area + joint_y * feature_map_size + joint_x;
            int offset_location_y = location_x + feature_map_area;
            LIBMAIX_DEBUG_PRINTF();
            float offset_x = offsets[offset_location_x];
            float offset_y = offsets[offset_location_y];
            LIBMAIX_DEBUG_PRINTF();
            float temp_res_x =(float) joint_x / feature_map_size;
            float temp_res_y = (float) joint_y / feature_map_size;

            LIBMAIX_DEBUG_PRINTF();

            // printf("temp(x:%f , y:%f) , score:%f\n", temp_res_x ,temp_res_y , score);
            if (temp_res_x < score)
            {
                LIBMAIX_DEBUG_PRINTF();
                params->result->keypoints[n * 2] = -1;
            }
            else
            {
                LIBMAIX_DEBUG_PRINTF();
                int a = float_to_int(temp_res_x * 240);
                params->result->keypoints[n * 2] =a ;
                LIBMAIX_DEBUG_PRINTF();
            }

            if (temp_res_y < score)
            {
                LIBMAIX_DEBUG_PRINTF();
                params->result->keypoints[n * 2 + 1] = -1;
            }
            else
            {
                LIBMAIX_DEBUG_PRINTF();
                int b = float_to_int(temp_res_y * 240);
                params->result->keypoints[n * 2 + 1] =b;
            }
            LIBMAIX_DEBUG_PRINTF();

        }
        outer_result_object->keypoints = params->result->keypoints;
        outer_result_object->cx = params->result->cx;
        outer_result_object->cy = params->result->cy;
        return LIBMAIX_ERR_NONE;
    }


#ifdef __cplusplus
}
#endif
