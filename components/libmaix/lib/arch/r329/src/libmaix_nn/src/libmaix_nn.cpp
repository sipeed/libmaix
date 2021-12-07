#include "standard_api.h"
#include "libmaix_err.h"
#include "libmaix_nn.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <cstring>
#include <errno.h>
#include <vector>

#define FNAME_MAX_LEN 4096

using namespace std;

aipu_ctx_handle_t *ctx = NULL; //  a struct AIPU could use it straightly
const char *status_msg = NULL;

aipu_buffer_alloc_info_t buffer;
libmaix_err_t status = LIBMAIX_ERR_NONE;
aipu_graph_desc_t gdesc;
uint32_t job_id = 0;
int32_t time_out = -1;

int in_fsize = 0;



typedef struct obj_config
{
    aipu_buffer_alloc_info_t buffer;
    libmaix_err_t status;
    aipu_graph_desc_t gdesc;
    uint32_t job_id  ;
    int32_t time_out ;
    // char data_file_name[FNAME_MAX_LEN] = {0};
    int in_fsize ;
} obj_config_t;

int c = 0;

libmaix_err_t libmaix_nn_obj_init(struct libmaix_nn *obj)
{
    if (obj == NULL)
    {
        status = LIBMAIX_ERR_NO_MEM;
        printf("initing a nn object is faild\n");
        return status;
    }
    ((obj_config_t *)(obj->_config))->status = status;
    ((obj_config_t *)(obj->_config))->buffer = buffer;
    ((obj_config_t *)(obj->_config))->gdesc = gdesc;
    ((obj_config_t *)(obj->_config))->in_fsize = in_fsize;
    ((obj_config_t *)(obj->_config))->job_id = job_id;
    ((obj_config_t *)(obj->_config))->time_out = time_out;

    libmaix_err_t status = ((obj_config_t *)(obj->_config))->status;
    aipu_status_t ret;

    return status;
}

libmaix_err_t libmaix_nn_obj_deinit(struct libmaix_nn *obj)
{
    free(obj->_config);
    free(obj);
    if (obj->_config== NULL ||  obj == NULL)
    {
        return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
}

libmaix_err_t libmaix_nn_obj_load(struct libmaix_nn *obj, const libmaix_nn_model_path_t *path, libmaix_nn_opt_param_t *opt_param)
{
    libmaix_err_t *status = &(((obj_config_t *)(obj->_config))->status);
    aipu_graph_desc_t *gdesc_ptr = &(((obj_config_t *)(obj->_config))->gdesc);
    aipu_buffer_alloc_info_t * buffer_ptr =  &(((obj_config_t *)(obj->_config))->buffer);
    aipu_status_t ret;

    if (path->normal.model_path == NULL)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        printf("[libmaix_nn]--  normal model is not implement\n");
    }

    // printf("[libmaix_nn ]--  the model path is: %s \n", path->normal.model_path);
    // printf("[libmaix_nn] -- start load graph\n");
    ret = AIPU_load_graph_helper(ctx, path->normal.model_path, gdesc_ptr);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        printf("[libmaix_nn ]  -- load_graph_error\n");
        fprintf(stderr, "[TEST ERROR] AIPU_load_graph_helper: %s\n", status_msg);
        
        ret = AIPU_deinit_ctx(ctx);
        if (ret != AIPU_STATUS_SUCCESS)
         {
             AIPU_get_status_msg(ret, &status_msg);
            printf("[DEMO ERROR] AIPU_deinit_ctx: %s\n", status_msg);
        }
        return *status;
    }
    // printf("[libmaix_nn] -- start load graph has done\n");
    fprintf(stdout, "[TEST INFO] AIPU load graph successfully.\n");



    printf("[libmaix_nn] -- start alloction tensor buffers\n");
    ret = AIPU_alloc_tensor_buffers(ctx,gdesc_ptr,buffer_ptr);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stdout, "[TEST ERROR] AIPU_alloc_tensor_buffers: %s\n", status_msg);

        printf("start to unload graph\n");
        ret = AIPU_unload_graph(ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stdout, "[TEST ERROR] AIPU_unload_graph; %s\n", status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        ret = AIPU_deinit_ctx(ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }
    }
    return *status;
}

libmaix_err_t libmaix_nn_obj_forward(struct libmaix_nn *obj, libmaix_nn_layer_t *inputs, libmaix_nn_layer_t *outputs)
{
    libmaix_err_t *status = &(((obj_config_t *)(obj->_config))->status);
    aipu_status_t ret;

    aipu_graph_desc_t *gdesc_ptr = &(((obj_config_t *)(obj->_config))->gdesc);
    aipu_buffer_alloc_info_t *buffer_ptr = &(((obj_config_t *)(obj->_config))->buffer);

    int model_inw = gdesc_ptr->inputs.desc[0].fmt.shape.W;
    int model_inh = gdesc_ptr->inputs.desc[0].fmt.shape.H;
    int model_inch = gdesc_ptr->inputs.desc[0].fmt.shape.C;
    // printf("[libmaix_nn]--   Model input:  W=%3d, H=%3d, C =%d, size=%d\r\n", model_inw, model_inh, model_inch, (*buffer_ptr).inputs.tensors[0].size);
    int img_size = model_inw * model_inh * model_inch;
    ((obj_config_t *)(obj->_config))->in_fsize = img_size;

    if(inputs->need_quantization == true)
    {
        int size = (inputs->h * inputs->w * inputs->c);
        uint8_t * pixels = (uint8_t *) inputs->data;
        int8_t *quant_data = (int8_t *)malloc(sizeof(int8_t) * size);
        for(int i=0 ; i < size ;i++)
        {
            quant_data[i] = pixels[i] - 127;
        }
        inputs->data = quant_data;
        memcpy((*buffer_ptr).inputs.tensors[0].va,  inputs->data, (*buffer_ptr).inputs.tensors[0].size);
        free(quant_data);
    }
    else
    {
        memcpy((*buffer_ptr).inputs.tensors[0].va,  inputs->data, (*buffer_ptr).inputs.tensors[0].size);
    }
    

    ret = AIPU_create_job(ctx, gdesc_ptr, (*buffer_ptr).handle, &(((obj_config_t *)(obj->_config))->job_id));

    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_create_job: %s\n", status_msg);
        printf("Create process jdb faild\n");

        printf("Start seting tensor buffers free\n");
        ret = AIPU_free_tensor_buffers(ctx,buffer_ptr->handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n", status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
        }

        ret = AIPU_unload_graph(ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stdout, "[TEST ERROR] AIPU_unload_graph; %s\n", status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        ret = AIPU_deinit_ctx(ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }
    }


    ret = AIPU_finish_job(ctx, ((obj_config_t *)(obj->_config))->job_id, ((obj_config_t *)(obj->_config))->time_out);

    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_finish_job: %s\n", status_msg);

         ret = AIPU_free_tensor_buffers(ctx,buffer_ptr->handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n", status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
        }

        ret = AIPU_unload_graph(ctx,gdesc_ptr);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stdout, "[TEST ERROR] AIPU_unload_graph; %s\n", status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        ret = AIPU_deinit_ctx(ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
            printf("deinit nn module is faild\n");
            return *status;
        }

        ret = AIPU_clean_job(ctx, ((obj_config_t *)(obj->_config))->job_id);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_clean_job: %s\n", status_msg);
            printf("clean job is faild\n");
            return *status;
        }

    }

    if(outputs->dtype == LIBMAIX_NN_DTYPE_FLOAT)
    {
        float Scale = 7.539542;    // the dequantize scale is fixed .in the next step the scale should be given from AIPU API.
        int size = (*buffer_ptr).outputs.tensors[0].size;
        int8_t* data = (int8_t *)((*buffer_ptr).outputs.tensors[0].va);
        float *prediction = (float *)outputs->data;
        for(int i=0 ; i < size ; i++)
        {
            prediction[i] = data[i] / Scale;
        }
        outputs->data = prediction;
    }
    else{
        
        memcpy(outputs->data, (int8_t *)((*buffer_ptr).outputs.tensors[0].va), (*buffer_ptr).outputs.tensors[0].size);
    }

    ret = AIPU_clean_job(ctx, ((obj_config_t *)(obj->_config))->job_id);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_clean_job: %s\n", status_msg);
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
    nn_obj_ptr->_config = obj_config_ptr;
    return nn_obj_ptr;
}

void libmaix_nn_destroy(libmaix_nn_t **obj)
{
    *obj = NULL;
}

libmaix_err_t libmaix_nn_module_init()
{
    libmaix_err_t status = LIBMAIX_ERR_NONE;
    aipu_status_t ret;
    ret = AIPU_init_ctx(&ctx);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_INIT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_init_ctx: %s\n", status_msg);
        printf("nn module init is faild\n");
        return status;
    }
    return status;
}

libmaix_err_t libmaix_nn_module_deinit()
{
    aipu_status_t ret;
    libmaix_err_t status = LIBMAIX_ERR_NONE;
    ret = AIPU_deinit_ctx(ctx);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_UNKNOWN;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
        printf("nn module deinit is faild;\n");
        return status;
    }
}
