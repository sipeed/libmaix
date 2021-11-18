#include "standard_api.h"
#include "libmaix_err.h"
#include "libmaix_nn.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <errno.h>
#include "standard_api.h"

#define FNAME_MAX_LEN 4096
libmaix_err_t status = LIBMAIX_ERR_NONE;
aipu_status_t ret = AIPU_STATUS_SUCCESS;
aipu_ctx_handle_t *ctx = NULL; //  a struct AIPU could use it straightly
const char *status_msg = NULL;

// need to transform
aipu_simulation_config_t config; //struct
aipu_graph_desc_t gdesc;
aipu_buffer_alloc_info_t buffer;
uint32_t job_id = 0;
int32_t time_out = -1;

extern char *optarg;
int opt_idx = 0;
int c = 0;
char simulator[FNAME_MAX_LEN] = {0};
char cfg_file_dir[FNAME_MAX_LEN] = {0};
char bin_file_name[FNAME_MAX_LEN] = {0};
char data_file_name[FNAME_MAX_LEN] = {0};
char check_file_name[FNAME_MAX_LEN] = {0};
char dump_dir[FNAME_MAX_LEN] = {0};

void *in_data = NULL;
int in_fsize = 0;
void *check_data = NULL;
int check_fsize = 0;

/* not Comple*/
libmaix_err_t libmaix_nn_obj_init(struct libmaix_nn *obj)
{
    if (status != LIBMAIX_ERR_NONE)
    {
        print("new an nn object is faild!\n");
        return status;
    }
    obj = (libmaix_nn_t *)malloc(1 * sizeof(libmaix_err_t));

    if (obj == NULL)
    {
        status = LIBMAIX_ERR_NO_MEM;
        print("allocation memory to an nn object is faild\n");
        return status;
    }
    return status;
}
libmaix_err_t libmaix_nn_obj_deinit(struct libmaix_nn *obj)
{
    free(obj);
}

/*
TODO：
1.from nn.h datastruct transform to AIPU's  structer 
*/
libmaix_err_t libmaix_nn_obj_load(struct libmaix_nn *obj, const libmaix_nn_model_path_t *path, libmaix_nn_opt_param_t *opt_param)
{
    /*
    TODO:
    transform out information to AIPU information structer
    */
    status = LIBMAIX_ERR_NONE;
    // load graph (aipu.bin)
    ret = AIPU_load_graph_helper(ctx, bin_file_name, &gdesc);

    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        printf("load_graph_error\n");
        fprintf(stderr, "[TEST ERROR] AIPU_load_graph_helper: %s\n", status_msg);
        return status;
    }
    fprintf(stdout, "[TEST INFO] AIPU load graph successfully.\n");

    //alloc tensor buffers
    ret = AIPU_alloc_tensor_buffers(ctx, &gdesc, &buffer);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        // alloc tensor faild
        status = LIBMAIX_ERR_NOT_READY;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_alloc_tensor_buffers: %s\n", status_msg);
        printf("allocateing tensor buffers  is faild!\n");

        //
        printf("start to unload graph\n");
        ret = AIPU_unload_graph(ctx, &gdesc);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_unload_graph; %s\n", status_msg);
            printf(" Unload graph is faild\n");
            return status;
        }

        // deinit nn module
        ret = AIPU_deinit_ctx(ctx);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            status = LIBMAIX_ERR_NOT_READY;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_deinit_ctx: %s\n", status_msg);
            printf("deinit nn module is faild\n");
            return status;
        }
    }
    return status;
}

libmaix_err_t libmaix_nn_obj_forward(struct libmaix_nn *obj, libmaix_nn_layer_t *inputs, libmaix_nn_layer_t *outputs)
{
    //add input data
    status = LIBMAIX_ERR_NONE;
    in_data = load_data_from_file(data_file_name, &in_fsize);
    if (!in_data)
    {
        status = LIBMAIX_ERR_PARAM;
        fprintf(stdout, "[TEST ERROR] Load input from file failed!\n");
        printf("load input data faild\n");
        return status;
    }
    memcpy(buffer.inputs.tensors[0].va, in_data, buffer.inputs.tensors[0].size);

    // ceate  process job
    ret = AIPU_create_job(ctx, &gdesc, buffer.handle, &job_id);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_create_job: %s\n", status_msg);
        pritnf("Create process jdb faild\n");

        printf("Start seting tensor buffers free\n");
        ret = AIPU_free_tensor_buffers(ctx, buffer.handle);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_free_tensor_buffers: %s\n", status_msg);
            printf("free tensor buffers is faild\n");
            // free input data memory
            if (in_data)
            {
                munmap(in_data, in_fsize);
            }
            return status;
        }
    }
    // pass  the input data  comput

    ret = AIPU_finish_job(ctx, job_id, time_out);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_IMPLEMENT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_finish_job: %s\n", status_msg);
        printf("finish job is faild\n");

        // 手动清除 job
        printf("Start cleaning job\n");
        ret = AIPU_clean_job(ctx, job_id);
        if (ret != AIPU_STATUS_SUCCESS)
        {
            status = LIBMAIX_ERR_NOT_IMPLEMENT;
            AIPU_get_status_msg(ret, &status_msg);
            fprintf(stderr, "[TEST ERROR] AIPU_clean_job: %s\n", status_msg);
            printf("clean job is faild\n");
            return status;
        }
    }
    return status;
}

libmaix_nn_t *libmaix_nn_create()
{

    //just  only a statement
    libmaix_nn_t *nn_obj_ptr;
    nn_obj_ptr->init = libmaix_nn_obj_init;
    nn_obj_ptr->deinit = libmaix_nn_obj_deinit;
    nn_obj_ptr->load = libmaix_nn_obj_load;
    nn_obj_ptr->forward = libmaix_nn_obj_forward;
    return nn_obj_ptr;
}

void libmaix_nn_destory(libmaix_nn_t **obj)
{
    *obj = NULL;
}

/*
TODO:
    initialize AIPU UMD context
    1.from nn.h datastruct transform to AIPU's  structer 
*/
libmaix_err_t libmaix_nn_module_init()
{
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

    config.simulator = simulator;
    config.cfg_file_dir = cfg_file_dir;
    config.output_dir = dump_dir;
    config.simulator_opt = NULL;

    ret = AIPU_config_simulation(ctx, &config);
    if (ret != AIPU_STATUS_SUCCESS)
    {
        status = LIBMAIX_ERR_NOT_INIT;
        AIPU_get_status_msg(ret, &status_msg);
        fprintf(stderr, "[TEST ERROR] AIPU_config_simulation: %s\n", status_msg);
        printf("set simulation is faild\n");
        return status;
    }

    return status;
}

libmaix_err_t liabmaix_nn_module_deinit()
{
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
