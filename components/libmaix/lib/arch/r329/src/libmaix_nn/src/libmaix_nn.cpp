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

aipu_ctx_handle_t *ctx = nullptr; //  a struct AIPU could use it straightly
const char *status_msg = nullptr;

// need to transform

typedef struct obj_config
{
    aipu_buffer_alloc_info_t buffer;
    libmaix_err_t status = LIBMAIX_ERR_NONE;
    aipu_graph_desc_t gdesc;
    uint32_t job_id = 0;
    int32_t time_out = -1;
    // char data_file_name[FNAME_MAX_LEN] = {0};
    int in_fsize = 0;
} obj_config_t;

int c = 0;

// static void *load_data_from_file(const char *fname, int *size)
// {
//     int fd = 0;
//     struct stat finfo;
//     void *start = nullptr;

//     if ((nullptr == fname) || (nullptr == size))
//     {
//         goto finish;
//     }

//     if (stat(fname, &finfo) != 0)
//     {
//         goto finish;
//     }

//     fd = open(fname, O_RDONLY);
//     if (fd <= 0)
//     {
//         fprintf(stderr, "open file failed: %s! (errno = %d)\n", fname, errno);
//         goto finish;
//     }

//     start = mmap(nullptr, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
//     if (MAP_FAILED == start)
//     {
//         fprintf(stderr, "failed in mapping graph file: %s! (errno = %d)\n", fname, errno);
//         goto finish;
//     }

//     /* success */
//     *size = finfo.st_size;

// finish:
//     if (fd > 0)
//     {
//         close(fd);
//     }
//     return start;
// }

/* not Comple*/
libmaix_err_t libmaix_nn_obj_init(struct libmaix_nn *obj)
{
    libmaix_err_t *status = &((obj_config_t *)obj->_config)->status;
    aipu_status_t ret;

    obj = (libmaix_nn_t *)malloc(1 * sizeof(libmaix_err_t));

    if (obj == NULL)
    {
        *status = LIBMAIX_ERR_NO_MEM;
        printf("allocation memory to an nn object is faild\n");
        return *status;
    }
    //alloc tensor buffers

    ret = AIPU_alloc_tensor_buffers(ctx, &(((obj_config_t *)(obj->_config))->gdesc), &(((obj_config_t *)(obj->_config))->buffer));
    if (ret != AIPU_STATUS_SUCCESS)
    {
        // alloc tensor faild
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_alloc_tensor_buffers: %s\n", status_msg);
        printf("allocateing tensor buffers  is faild!\n");

        //
        printf("start to unload graph\n");
        ret = AIPU_unload_graph(ctx, &(((obj_config_t *)(obj->_config))->gdesc));
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_unload_graph; %s\n", status_msg);
            printf(" Unload graph is faild\n");
            return *status;
        }

        // deinit nn module
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

libmaix_err_t libmaix_nn_obj_deinit(struct libmaix_nn *obj)
{
    free(obj);
}

libmaix_err_t libmaix_nn_obj_load(struct libmaix_nn *obj, const libmaix_nn_model_path_t *path, libmaix_nn_opt_param_t *opt_param)
{
    libmaix_err_t *status = &((obj_config_t *)obj->_config)->status;
    aipu_status_t ret;
    if (path->normal.model_path != NULL)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        printf("normal model is not implement\n");
    }

    // load graph (aipu.bin)
    ret = AIPU_load_graph_helper(ctx, path->awnn.bin_path, &(((obj_config_t *)(obj->_config))->gdesc));

    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        printf("load_graph_error\n");
        fprintf(stderr, "[TEST ERROR] AIPU_load_graph_helper: %s\n", status_msg);
        return *status;
    }
    fprintf(stdout, "[TEST INFO] AIPU load graph successfully.\n");

    return *status;
}

libmaix_err_t libmaix_nn_obj_forward(struct libmaix_nn *obj, libmaix_nn_layer_t *inputs, libmaix_nn_layer_t *outputs)
{
    libmaix_err_t *status = &((obj_config_t *)obj->_config)->status;
    aipu_status_t ret;
    // void * in_data = load_data_from_file(          ((obj_config_t *) (obj->_config))->data_file_name    ,  &(((obj_config_t *) (obj->_config))->in_fsize ));
    // if (!in_data)
    // {
    //     *status = LIBMAIX_ERR_PARAM;
    //     fprintf(stdout, "[TEST ERROR] Load input from file failed!\n");
    //     printf("load input data faild\n");
    //     return *status;
    // }

    memcpy((*(aipu_buffer_alloc_info_t *)obj->_config).inputs.tensors[0].va, inputs->data, (*(aipu_buffer_alloc_info_t *)obj->_config).inputs.tensors[0].size);

    *status = LIBMAIX_ERR_NONE;
    // ceate  process job
    ret = AIPU_create_job(ctx, &(((obj_config_t *)(obj->_config))->gdesc), (((obj_config_t *)(obj->_config))->buffer.handle), &(((obj_config_t *)(obj->_config))->job_id));
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_create_job: %s\n", status_msg);
        printf("Create process jdb faild\n");

        printf("Start seting tensor buffers free\n");
        ret = AIPU_free_tensor_buffers(ctx, (*(aipu_buffer_alloc_info_t *)obj->_config).handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            *status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n", status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
            if (inputs->data)
            {
                munmap(inputs->data, (*(aipu_buffer_alloc_info_t *)obj->_config).inputs.tensors[0].size);
            }
            return *status;
        }
    }
    // pass  the input data  comput

    ret = AIPU_finish_job(ctx, ((obj_config_t *)(obj->_config))->job_id, ((obj_config_t *)(obj->_config))->time_out);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        *status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_finish_job: %s\n", status_msg);
        printf("finish job is faild\n");

        // 手动清除 job
        printf("Start cleaning job\n");
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

    memcpy(outputs->data, (*(aipu_buffer_alloc_info_t *)obj->_config).inputs.tensors[0].va, (*(aipu_buffer_alloc_info_t *)obj->_config).outputs.tensors[0].size);

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
    nn_obj_ptr->init = libmaix_nn_obj_init;
    nn_obj_ptr->deinit = libmaix_nn_obj_deinit;
    nn_obj_ptr->load = libmaix_nn_obj_load;
    nn_obj_ptr->forward = libmaix_nn_obj_forward;
    // made a struct to add
    obj_config_t obj_config;
    nn_obj_ptr->_config = &obj_config;
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
    aipu_runtime_config_t config; //struct
    ret = AIPU_set_runtime_config(ctx, &config);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_INIT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_init_ctx: %s\n", status_msg);
        printf("set runtime config is faild!\n");
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
