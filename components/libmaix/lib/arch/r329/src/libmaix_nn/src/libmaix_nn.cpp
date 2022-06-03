
#include "libmaix_err.h"
#include "libmaix_nn.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <errno.h>
#include "stdlib.h"
#include <string.h>
#include "standard_api.h"

#define debug_line //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)

typedef struct obj_config
{
    aipu_buffer_alloc_info_t buffer;
    libmaix_err_t status;
    aipu_graph_desc_t gdesc;
    uint32_t job_id  ;
    int32_t time_out ;
    aipu_ctx_handle_t * ctx ;
    const char *status_msg;
    libmaix_nn_opt_param_t * opt;

} obj_config_t;

libmaix_err_t libmaix_nn_obj_init(struct libmaix_nn *obj)
{
    ((obj_config_t *)(obj->config))->status = LIBMAIX_ERR_NONE;
    libmaix_err_t * status = &( ((obj_config_t *)(obj->config))->status);
    // libmaix_err_t status =  LIBMAIX_ERR_NONE;
    if (obj == NULL)
    {
        *status = LIBMAIX_ERR_NO_MEM;
        printf("initing a nn object is faild\n");
        return *status;
    }
    ((obj_config_t *)(obj->config))->status = LIBMAIX_ERR_NONE;
    // ((obj_config_t *)(obj->config))->job_id = 0;
    ((obj_config_t *)(obj->config))->time_out = -1;
    ((obj_config_t *)(obj->config))->ctx  = NULL;
    ((obj_config_t *)(obj->config))->status_msg = NULL;
    ((obj_config_t *)(obj->config))->opt = NULL;
    aipu_status_t ret;

    ret = AIPU_init_ctx(&((obj_config_t *)(obj->config))->ctx);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_INIT;
        // const char *status_msg =  ((obj_config_t *)(obj->config))->status_msg;
        AIPU_get_status_msg(ret, &((obj_config_t *)(obj->config))->status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_init_ctx: %s\n", ((obj_config_t *)(obj->config))->status_msg);
        printf("nn module init is faild\n");
        return *status;
    }
    return *status;
}

libmaix_err_t libmaix_nn_obj_deinit(struct libmaix_nn *obj)
{

    if ( ((obj_config_t *)(obj->config))->status != LIBMAIX_ERR_NONE)
    {
        return LIBMAIX_ERR_NOT_IMPLEMENT;
    }
    else
    {
        // aipu_ctx_handle_t *ctx  = ((obj_config_t *)(obj->config))->ctx;
        // const char *status_msg =  ((obj_config_t *)(obj->config))->status_msg;
        aipu_status_t ret;
        libmaix_err_t *status = & ((obj_config_t *)(obj->config))->status;
        aipu_ctx_handle_t ** ctx = &(((obj_config_t *)(obj->config))->ctx);
        const char ** status_msg = &(((obj_config_t *)(obj->config))->status_msg);

        ret = AIPU_deinit_ctx(*ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_UNKNOWN;
            AIPU_get_status_msg(ret,status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", *status_msg);
            printf("nn module deinit is faild;\n");
            return *status;
        }
        return *status;
    }
}

libmaix_err_t libmaix_nn_obj_load(struct libmaix_nn *obj, const libmaix_nn_model_path_t *path, libmaix_nn_opt_param_t *opt_param)
{
    libmaix_err_t *status = &(((obj_config_t *)(obj->config))->status);
    aipu_graph_desc_t *gdesc_ptr = &(((obj_config_t *)(obj->config))->gdesc);
    aipu_buffer_alloc_info_t * buffer_ptr =  &(((obj_config_t *)(obj->config))->buffer);

    aipu_ctx_handle_t ** ctx = &(((obj_config_t *)(obj->config))->ctx);
    const char ** status_msg = &(((obj_config_t *)(obj->config))->status_msg);

    aipu_status_t ret;

    // aipu_ctx_handle_t *ctx  = ((obj_config_t *)(obj->config))->ctx;
    // const char *status_msg =  ((obj_config_t *)(obj->config))->status_msg;
    ((obj_config_t *)(obj->config))->opt = opt_param;
    ((obj_config_t *)(obj->config))->opt = (libmaix_nn_opt_param_t *) malloc( sizeof(libmaix_nn_opt_param_t));
    *((obj_config_t *)(obj->config))->opt   = * opt_param;
    if (path->aipu.model_path == NULL)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        printf("[libmaix_nn]--  aipu model can't be opened \n");
    }

    ret = AIPU_load_graph_helper(*ctx ,path->aipu.model_path, gdesc_ptr);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, status_msg);
        printf("[libmaix_nn ]  -- load_graph_error\n");
        fprintf(stderr, "[TEST ERROR] AIPU_load_graph_helper: %s\n",*status_msg);

        ret = AIPU_deinit_ctx(*ctx);
        if (ret != AIPU_STATUS_SUCCESS)
         {
             AIPU_get_status_msg(ret, status_msg);
            printf("[DEMO ERROR] AIPU_deinit_ctx: %s\n",*status_msg);
        }
        return *status;
    }
    fprintf(stdout, "[TEST INFO] AIPU load graph successfully.\n");

    printf("[libmaix_nn] -- start alloction tensor buffers\n");
    ret = AIPU_alloc_tensor_buffers(*ctx,gdesc_ptr,buffer_ptr);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, status_msg);
        fprintf(stdout, "[TEST ERROR] AIPU_alloc_tensor_buffers: %s\n", *status_msg);

        printf("start to unload graph\n");
        ret = AIPU_unload_graph(*ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret,status_msg);
            fprintf(stdout, "[TEST ERROR] AIPU_unload_graph; %s\n", *status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        ret = AIPU_deinit_ctx(*ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", *status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }
    }
    return *status;
}

libmaix_err_t libmaix_nn_obj_forward(struct libmaix_nn *obj, libmaix_nn_layer_t *inputs, libmaix_nn_layer_t *outputs)
{
    // printf("[libmaix nn ]  forward\n");
    libmaix_err_t *status = &(((obj_config_t *)(obj->config))->status);
    aipu_status_t ret;

    aipu_graph_desc_t *gdesc_ptr = &(((obj_config_t *)(obj->config))->gdesc);
    aipu_buffer_alloc_info_t *buffer_ptr = &(((obj_config_t *)(obj->config))->buffer);
    aipu_ctx_handle_t ** ctx = &(((obj_config_t *)(obj->config))->ctx);
    const char ** status_msg = &(((obj_config_t *)(obj->config))->status_msg);

    // aipu_ctx_handle_t *ctx  = ((obj_config_t *)(obj->config))->ctx;
    // const char *status_msg =  ((obj_config_t *)(obj->config))->status_msg;

    int model_inw = gdesc_ptr->inputs.desc[0].fmt.shape.W;
    int model_inh = gdesc_ptr->inputs.desc[0].fmt.shape.H;
    int model_inch = gdesc_ptr->inputs.desc[0].fmt.shape.C;
    // printf("[libmaix_nn]--   Model input:  W=%3d, H=%3d, C =%d, size=%d\r\n", model_inw, model_inh, model_inch, (*buffer_ptr).inputs.tensors[0].size);
    int size = inputs->h * inputs->w;
    float R = ((obj_config_t *)(obj->config))->opt->aipu.mean[0];
    float G = ((obj_config_t *)(obj->config))->opt->aipu.mean[1];
    float B = ((obj_config_t *)(obj->config))->opt->aipu.mean[2];
    float norm_R = ((obj_config_t *)(obj->config))->opt->aipu.norm[0];
    float norm_G = ((obj_config_t *)(obj->config))->opt->aipu.norm[1];
    float norm_B = ((obj_config_t *)(obj->config))->opt->aipu.norm[2];
    uint8_t * pixels = (uint8_t *) inputs->data;
    if(inputs->need_quantization == true)
    {
        debug_line;
        int8_t * temp_buffer =(int8_t *) (inputs->buff_quantization);
        if(inputs->buff_quantization == NULL )
        {
            debug_line;
             printf("[libmaix_nn] --  input has not init quantization buffer\n");
            int8_t *quant_data = (int8_t *)malloc(sizeof(int8_t) * size * 3);
            for(int i=0 ; i != size ;i ++)
            {
                quant_data[i *3 + 0] = pixels[i * 3 + 0] - R;
                quant_data[i *3 + 1] = pixels[i * 3 + 1] - G;
                quant_data[i *3 + 2] = pixels[i * 3 + 2] - B;
            }
            memcpy((*buffer_ptr).inputs.tensors[0].va,  quant_data, (*buffer_ptr).inputs.tensors[0].size);
            free(quant_data);
        }
        else
        {
            debug_line;
            for(int i=0 ; i < size ;i++)
            {
                temp_buffer[i *3 + 0] = pixels[i * 3+ 0] - R;
                temp_buffer[i *3 + 1] = pixels[i * 3+ 1] - G;
                temp_buffer[i *3 + 2] = pixels[i * 3+ 2] - B;
            }
            debug_line;
            memcpy((*buffer_ptr).inputs.tensors[0].va, temp_buffer, (*buffer_ptr).inputs.tensors[0].size);
        }

    }
    else
    {
        debug_line;
        float *temp_float_buffer = (float *)malloc(sizeof(float) * size *  3);
        debug_line;
        for(int i=0 ; i < size ;i++)
        {
            int8_t int8_temp_R = pixels[i * 3+ 0] - R;
            int8_t int8_temp_G = pixels[i * 3+ 1] - G;
            int8_t int8_temp_B = pixels[i * 3+ 2] - B;
            // debug_line;
            float temp_R = (float)int8_temp_R;
            float temp_G = (float)int8_temp_G;
            float temp_B = (float)int8_temp_B;
            // debug_line;
            temp_float_buffer[i *3 + 0] =  temp_R * norm_R;
            temp_float_buffer[i *3 + 1] =  temp_G * norm_G;
            temp_float_buffer[i *3 + 2] =  temp_B * norm_B;
            // debug_line;
        }
        debug_line;
        memcpy((*buffer_ptr).inputs.tensors[0].va, temp_float_buffer, (*buffer_ptr).inputs.tensors[0].size);
        debug_line;
        free(temp_float_buffer);
    }
    debug_line;
    uint32_t * job_id = &(((obj_config_t *)(obj->config))->job_id);
    // ret = AIPU_create_job(ctx, gdesc_ptr, (*buffer_ptr).handle, &(((obj_config_t *)(obj->config))->job_id));
    // printf("[libmaix_nn] --  ready create job\n");

    ret = AIPU_create_job(*ctx, gdesc_ptr, (*buffer_ptr).handle, job_id);
    // printf("[libmaix_nn] --  create job has down \n");

    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret,status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_create_job: %s\n", *status_msg);
        printf("Create process jdb faild\n");

        printf("Start seting tensor buffers free\n");
        ret = AIPU_free_tensor_buffers(*ctx,buffer_ptr->handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret,status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n",*status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
        }

        ret = AIPU_unload_graph(*ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stdout, "[TEST ERROR] AIPU_unload_graph; %s\n", *status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        ret = AIPU_deinit_ctx(*ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n",*status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }
    }
    // printf("[libmaix_nn]-- the job id is %d \n",&(((obj_config_t *)(obj->config))->job_id));
    // ret = AIPU_finish_job(ctx, ((obj_config_t *)(obj->config))->job_id, ((obj_config_t *)(obj->config))->time_out);
    // printf("[libmaix_nn] --  ready finish job \n");
    debug_line;
    ret = AIPU_finish_job(*ctx, *job_id, ((obj_config_t *)(obj->config))->time_out);
    debug_line;
    // printf("[libmaix_nn] --  ready finish job  is done\n");

    if (ret != AIPU_STATUS_SUCCESS)
    {
        printf("[libmaix_nn] --  ready finish job  is faild\n");

        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_finish_job: %s\n", *status_msg);

         ret = AIPU_free_tensor_buffers(*ctx,buffer_ptr->handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n",*status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
        }
        debug_line;
        ret = AIPU_unload_graph(*ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", *status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }
        debug_line;

        ret = AIPU_clean_job(*ctx, *job_id);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_clean_job: %s\n", *status_msg);
            printf("clean job is faild\n");
            return *status;
        }

    }

    uint8_t output_num = ((obj_config_t *)(obj->config))->opt->aipu.output_num;
    for (int out_id = 0 ; out_id < output_num ; out_id++)
    {

        // printf("[liamaix_nn]  out_id :%d \n" ,out_id);

        if(outputs[out_id].dtype == LIBMAIX_NN_DTYPE_FLOAT)
        {
            float scale = ((obj_config_t *)(obj->config))->opt->aipu.scale[out_id];

            // printf("[libmaix_nn ]dequantize scale :%f\n",scale);

            int size = (*buffer_ptr).outputs.tensors[out_id].size;
            // printf("[libmaix_nn ]  output :%d   size: %d \n" ,out_id,size);

            int8_t* data = (int8_t *)((*buffer_ptr).outputs.tensors[out_id].va);

            if(outputs[out_id].data == NULL)
            {
                printf("output feature map is not init \n");
                return  LIBMAIX_ERR_NOT_INIT;
            }

            float * temp =(float *) outputs[out_id].data;
            float ** prediction = &temp;
            for(int i=0 ; i < size ; i++)
            {
                // prediction[i] = data[i] / scale;
                (*prediction) [i] = data[i] /scale;
            }
        }
        else
        {
            memcpy(outputs[out_id].data , (int8_t *)((*buffer_ptr).outputs.tensors[out_id].va) ,   (*buffer_ptr).outputs.tensors[out_id].size);
        }
    }

    // // printf("[libmaix_nn]-- the job id is %d \n",&(((obj_config_t *)(obj->config))->job_id));

    // // ret = AIPU_clean_job(ctx, ((obj_config_t *)(obj->config))->job_id);
    ret = AIPU_clean_job(*ctx, *job_id);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_clean_job: %s\n", *status_msg);
        printf("clean job is faild\n");
        return *status;
    }
    return *status;
}

libmaix_nn_t *libmaix_nn_create()
{
    //just  only a statement
    libmaix_nn_t *nn_obj_ptr;
    nn_obj_ptr = (libmaix_nn_t *)malloc(1 * sizeof(libmaix_nn_t));
    nn_obj_ptr->init = libmaix_nn_obj_init;
    nn_obj_ptr->deinit = libmaix_nn_obj_deinit;
    nn_obj_ptr->load = libmaix_nn_obj_load;
    nn_obj_ptr->forward = libmaix_nn_obj_forward;
    // made a struct to add
    obj_config_t *obj_config_ptr = (obj_config_t *)malloc(1 * sizeof(obj_config_t));

    nn_obj_ptr->config = obj_config_ptr;
    return nn_obj_ptr;
}

void libmaix_nn_destroy(libmaix_nn_t **obj)
{
    if(*obj!= NULL)
    {
        free((*obj)->config);
        free(*obj);
    }
    *obj = NULL;

}

libmaix_err_t libmaix_nn_module_init()
{
    // aipu_status_t ret;
    // ret = AIPU_init_ctx(&ctx);
    // if (ret != AIPU_STATUS_SUCCESS)
    // {
    //     status = LIBMAIX_ERR_NOT_INIT;
    //     AIPU_get_status_msg(ret, &status_msg);
    //     fprintf(stderr, "[TEST ERROR] AIPU_init_ctx: %s\n", status_msg);
    //     printf("nn module init is faild\n");
    //     return status;
    // }
    return LIBMAIX_ERR_NONE;

}

libmaix_err_t libmaix_nn_module_deinit()
{
    // aipu_status_t ret;
    // libmaix_err_t status = LIBMAIX_ERR_NONE;
    // ret = AIPU_deinit_ctx(ctx);
    // if (ret != AIPU_STATUS_SUCCESS)
    // {
    //     status = LIBMAIX_ERR_UNKNOWN;
    //     AIPU_get_status_msg(ret, &status_msg);
    //     fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
    //     printf("nn module deinit is faild;\n");
    //     return status;
    // }

    return LIBMAIX_ERR_NONE;
}


float libmaix_nn_feature_compare_int8(int8_t* a, int8_t* b, int len)
{
    int count = 0;
    for(int index = 0; index < len ;index++ )
    {
        if(a[index] == b[index])
        {
            count++;
        }
    }
    float score = count / len ;
    return score ;
}
float libmaix_nn_feature_compare_float(float* a, float* b, int len)
{
    int count = 0;
    for(int index = 0; index < len ;index++ )
    {
        if(a[index] == b[index])
        {
            count++;
        }
    }
    float score = count / len ;
    return score ;
}
