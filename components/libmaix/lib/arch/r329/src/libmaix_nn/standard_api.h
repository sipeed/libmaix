/**********************************************************************************
 * This file is CONFIDENTIAL and any use by you is subject to the terms of the
 * agreement between you and Arm China or the terms of the agreement between you
 * and the party authorised by Arm China to disclose this file to you.
 * The confidential and proprietary information contained in this file
 * may only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm China.
 *
 *        (C) Copyright 2020 Arm Technology (China) Co. Ltd.
 *                    All rights reserved.
 *
 * This entire notice must be reproduced on all copies of this file and copies of
 * this file may only be made by a person if such person is permitted to do so
 * under the terms of a subsisting license agreement from Arm China.
 *
 *********************************************************************************/

/**
 * @file  standard_api.h
 * @brief AIPU User Mode Driver (UMD) Standard API header
 * @version 4.0
 */

#ifndef _STANDARD_API_H_
#define _STANDARD_API_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct ctx_handle aipu_ctx_handle_t;

/**
 * @brief AIPU tensor descriptions:
 *        format: layout/shape/data type
 *        buffer: address info
 */
typedef enum {
    TENSOR_LAYOUT_NONE = 0x0,
    TENSOR_LAYOUT_NHWC,
    TENSOR_LAYOUT_NCHW,
    TENSOR_LAYOUT_NWH,
    TENSOR_LAYOUT_NC
} aipu_tensor_layout_t;

typedef struct tensor_shape {
    uint32_t N;
    uint32_t H;
    uint32_t W;
    uint32_t C;
} aipu_tensor_shape_t;

typedef enum {
    TENSOR_DATA_TYPE_NONE = 0x0,
    TENSOR_DATA_TYPE_U8 = 0x1,
    TENSOR_DATA_TYPE_S8 = 0x2,
    TENSOR_DATA_TYPE_U16 = 0x3,
    TENSOR_DATA_TYPE_S16 = 0x4,
} aipu_data_type_t;

typedef struct aipu_tensor_fmt {
    aipu_tensor_layout_t layout;
    aipu_tensor_shape_t  shape;
    aipu_data_type_t     data_type;
} aipu_tensor_fmt_t;

typedef struct tensor_desc {
    uint32_t id;
    uint32_t size;
    aipu_tensor_fmt_t fmt;
} aipu_tensor_desc_t;

typedef struct buffer {
    uint32_t id;
    void*    va;
    uint32_t size;
} aipu_buffer_t;

/**
 * @brief AIPU tensor descriptions struct of specific types: input/output/dump
 *        tensor_info:   basic format info.
 *        tensor_buffer: buffer address info.
 */
typedef struct tensor_info {
    uint32_t number;
    const aipu_tensor_desc_t* desc;
} aipu_tensor_info_t;

typedef struct tensor_buffer {
    uint32_t number;
    const aipu_buffer_t* tensors;
} aipu_tensor_buffer_t;

/**
 * @brief AIPU loadable graph descriptions:
 *        graph_desc:   graph basic info. returned by AIPU_load_graph
 *        buffer_alloc: graph buffers info. returned by AIPU_alloc_tensor_buffers
 */
typedef struct graph_desc {
    uint32_t id;                     /**< graph ID */
    uint32_t graph_version;          /**< graph binary version number */
    uint8_t  building_tool_major;    /**< major number of the build_tool generating this binary */
    uint8_t  building_tool_minor;    /**< minor number of the build_tool generating this binary */
    uint16_t build_version;          /**< version number of the build_tool generating this binary */
    aipu_tensor_info_t inputs;       /**< struct contains input tensor descriptors info. */
    aipu_tensor_info_t outputs;      /**< struct contains output tensor descriptors info. */
    aipu_tensor_info_t inter_dumps;  /**< struct contains intermediate tensor descriptors info. */
} aipu_graph_desc_t;

typedef struct aipu_buffer_alloc_info {
    uint32_t handle;                     /**< buffer handle */
    aipu_tensor_buffer_t inputs;         /**< struct contains info. of input tensor buffers allocated */
    aipu_tensor_buffer_t outputs;        /**< struct contains info. of output tensor buffers allocated */
    aipu_tensor_buffer_t inter_dumps;    /**< struct contains info. of intermediate tensor buffers allocated */
    aipu_tensor_buffer_t printf_dumps;   /**< struct contains info. of printf log data buffers allocated */
    aipu_tensor_buffer_t profiler_dumps; /**< struct contains info. of profiling data buffers allocated */
} aipu_buffer_alloc_info_t;

/**
 * @brief AIPU simulation configuration; used only on x86-linux simulation platform.
 */
typedef struct simulation_config {
    const char* simulator;      /**< simulator executable path */
    const char* cfg_file_dir;   /**< simulator runtime config file path */
    const char* output_dir;     /**< simulator dump path */
    const char* simulator_opt;  /**< additional simulator options and arguments provided (if any) */
    uint64_t pa_base;           /**< base physical address for text, rodata and data buffers */
} aipu_simulation_config_t;

/**
 * @brief AIPU runtime configuration; used only on Arm-linux platform.
 */
typedef struct runtime_config {
    bool poll_opt; /**< poll optimization flag; should be enabled at multi-thread pipeline scanario */
    bool bypass_version_check; /**< flag used to bypass version checking between binary and hardware; by default disabled */
} aipu_runtime_config_t;

/**
 * @brief AIPU job status; returned by status querying API AIPU_get_job_end_status().
 */
typedef enum {
    AIPU_JOB_STATUS_NO_STATUS, /**< no status */
    AIPU_JOB_STATUS_DONE,      /**< job execution successfully */
    AIPU_JOB_STATUS_EXCEPTION  /**< job execution failed, encountering exception */
} aipu_job_status_t;

/**
 * @brief AIPU debugger job info struct; returned by UMD API for AIPU debugger to use
 */
typedef struct aipu_debugger_job_info {
    uint64_t instruction_base_pa; /**< instruction section base address (physical) */
    uint64_t start_pc_pa;         /**< start PC pointer address (physical) */
    uint64_t interrupt_pc_pa;     /**< interrupt handler base address (physical) */
} aipu_debugger_job_info_t;

/**
 * @brief AIPU core info struct; returned by UMD API for AIPU debugger to use
 */
typedef struct aipu_core_info {
    uint64_t reg_base; /**< core register base address */
} aipu_core_info_t;

/**
 * @brief AIPU memory dump flag; set by UMD application via API AIPU_set_dump_options()
 */
typedef enum {
    AIPU_DUMP_TEXT = 1 << 0,          /**< dump text section(s) */
    AIPU_DUMP_RO = 1 << 1,            /**< dump read-only data section(s) */
    AIPU_DUMP_STACK = 1 << 2,         /**< dump AIPU stack section(s) */
    AIPU_DUMP_STATIC_TENSOR = 1 << 3, /**< dump static data section(s) */
    AIPU_DUMP_REUSE_TENSOR = 1 << 4,  /**< dump reuse data section(s) */
    AIPU_DUMP_OUT_TENSOR = 1 << 5,    /**< dump output data tensor(s) */
    AIPU_DUMP_INTER_TENSOR = 1 << 6,  /**< dump intermediate tensor(s) */
    AIPU_DUMP_MEM_MAP = 1 << 7,       /**< dump memory allocation info */
    AIPU_DUMP_DRV_PROF_DATA = 1 << 8, /**< dump profiling data collected by kernel driver */
    AIPU_DUMP_BEFORE_RUN = 1 << 9,    /**< dump after loading & before job execution */
    AIPU_DUMP_AFTER_RUN = 1 << 10,    /**< dump after AIPU job execution done */
    AIPU_DUMP_MAX = 1 << 11           /**< dump flag max. value, invalid */
} aipu_dump_flag_t;

typedef struct dump_option {
    uint32_t flag;                    /**< dump flag combinations */
    const char* fname_suffix;         /**< dump file name suffix; set to be NULL if unused */
    const char* dir;                  /**< dump file path; set to be NULL if unused */
} aipu_dump_option_t;

/**
 * @brief This aipu_status_t enumeration captures the result of any API function
 *        that has been executed. Success is represented by AIPU_STATUS_SUCCESS
 *        which has a value of zero. Error statuses are assigned positive integers
 *        and their identifiers start with the AIPU_STATUS_ERROR prefix.
 */
typedef enum {
    AIPU_STATUS_SUCCESS                    = 0x0,
    AIPU_STATUS_ERROR_NULL_PTR             = 0x1,
    AIPU_STATUS_ERROR_INVALID_CTX          = 0x2,
    AIPU_STATUS_ERROR_OPEN_DEV_FAIL        = 0x3,
    AIPU_STATUS_ERROR_DEV_ABNORMAL         = 0x4,
    AIPU_STATUS_ERROR_DEINIT_FAIL          = 0x5,
    AIPU_STATUS_ERROR_INVALID_CONFIG       = 0x6,
    AIPU_STATUS_ERROR_GVERSION_UNSUPPORTED = 0x7,
    AIPU_STATUS_ERROR_TARGET_NOT_FOUND     = 0x8,
    AIPU_STATUS_ERROR_INVALID_GBIN         = 0x9,
    AIPU_STATUS_ERROR_GRAPH_NOT_EXIST      = 0xA,
    AIPU_STATUS_ERROR_OPEN_FILE_FAIL       = 0xB,
    AIPU_STATUS_ERROR_MAP_FILE_FAIL        = 0xC,
    AIPU_STATUS_ERROR_READ_FILE_FAIL       = 0xD,
    AIPU_STATUS_ERROR_WRITE_FILE_FAIL      = 0xE,
    AIPU_STATUS_ERROR_JOB_NOT_EXIST        = 0xF,
    AIPU_STATUS_ERROR_JOB_NOT_SCHED        = 0x10,
    AIPU_STATUS_ERROR_JOB_SCHED            = 0x11,
    AIPU_STATUS_ERROR_JOB_NOT_END          = 0x12,
    AIPU_STATUS_ERROR_JOB_EXCEPTION        = 0x13,
    AIPU_STATUS_ERROR_JOB_TIMEOUT          = 0x14,
    AIPU_STATUS_ERROR_INVALID_OPTIONS      = 0x15,
    AIPU_STATUS_ERROR_INVALID_PATH         = 0x16,
    AIPU_STATUS_ERROR_OP_NOT_SUPPORTED     = 0x17,
    AIPU_STATUS_ERROR_INVALID_OP           = 0x18,
    AIPU_STATUS_ERROR_INVALID_SIZE         = 0x19,
    AIPU_STATUS_ERROR_INVALID_HANDLE       = 0x1A,
    AIPU_STATUS_ERROR_BUSY_HANDLE          = 0x1B,
    AIPU_STATUS_ERROR_BUF_ALLOC_FAIL       = 0x1C,
    AIPU_STATUS_ERROR_BUF_FREE_FAIL        = 0x1D,
    AIPU_STATUS_ERROR_INVALID_CORE_ID      = 0x1E,
    AIPU_STATUS_ERROR_RESERVE_SRAM_FAIL    = 0x1F,
    AIPU_STATUS_MAX = 0x20
} aipu_status_t;

/**
 * @brief This API is used to query additional information about a status returned by UMD API
 *
 * @param status Status returned by UMD standard API
 * @param msg    Pointer to a memory location allocated by application where UMD stores the
 *               message string pointer
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 */
aipu_status_t AIPU_get_status_msg(aipu_status_t status, const char** msg);
/**
 * @brief This API is used to initialize AIPU UMD context
 *
 * @param[out] ctx Pointer to a memory location allocated by application where UMD stores the
 *                 opaque context handle struct
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_OPEN_DEV_FAIL
 * @retval AIPU_STATUS_ERROR_DEV_ABNORMAL
 */
aipu_status_t AIPU_init_ctx(aipu_ctx_handle_t** ctx);
/**
 * @brief This API is used to destroy AIPU UMD context
 *
 * @param[in] ctx Pointer to a context handle struct returned by AIPU_init_ctx
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_DEINIT_FAIL
 */
aipu_status_t AIPU_deinit_ctx(const aipu_ctx_handle_t* ctx);
/**
 * @brief This API is used to configure AIPU simulation platform
 *
 * @param[in] ctx    Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] config Pointer to a memory location allocated by application where stores the
 *                   configurations
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_INVALID_CONFIG
 * @retval AIPU_STATUS_ERROR_OP_NOT_SUPPORTED
 *
 * @note works only for x86-linux simulation platform
 */
aipu_status_t AIPU_config_simulation(const aipu_ctx_handle_t* ctx,
    const aipu_simulation_config_t* config);
/**
 * @brief This API is used to configure AIPU runtime execution options.
 *
 * @param[in] ctx    Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] config Pointer to a memory location allocated by application where stores the
 *                   runtime configurations
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 *
 * @note currently works only for Arm-linux simulation platform;
 * @note the config set via this API reflects from the next scheduled job's execution on;
 * @note by default all options/flags in config struct are not enabled in UMD;
 */
aipu_status_t AIPU_set_runtime_config(const aipu_ctx_handle_t* ctx,
    const aipu_runtime_config_t* config);
/**
 * @brief This API is used to load a graph binary for driver to parse and alloc static buffers
 *
 * @param[in]  ctx   Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  garph Pointer to a memory location allocated by application where stores the graph
 * @param[in]  size  Graph buffer size
 * @param[out] gdesc Pointer to a memory location allocated by application where UMD stores the
 *                   graph descriptor
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_GVERSION_UNSUPPORTED
 * @retval AIPU_STATUS_ERROR_TARGET_NOT_FOUND
 * @retval AIPU_STATUS_ERROR_INVALID_GBIN
 * @retval AIPU_STATUS_ERROR_RESERVE_SRAM_FAIL
 */
aipu_status_t AIPU_load_graph(const aipu_ctx_handle_t* ctx,
    const void* graph, uint32_t size, aipu_graph_desc_t* gdesc);
/**
 * @brief This API is a wrapper of AIPU_load_graph which loads an offline built graph binary
 *        file from file system.
 *
 * @param[in]  ctx        Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  garph_file Loadable graph binary file path
 * @param[out] gdesc      Pointer to a memory location allocated by application where UMD stores
 *                        the graph descriptor
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_OPEN_GBIN_FAIL
 * @retval AIPU_STATUS_ERROR_MAP_GBIN_FAIL
 * @retval Other values returned by AIPU_load_graph
 */
aipu_status_t AIPU_load_graph_helper(const aipu_ctx_handle_t* ctx,
    const char* graph_file, aipu_graph_desc_t* gdesc);
/**
 * @brief This API is used to unload a loaded graph
 *
 * @param[in] ctx   Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] gdesc Pointer to a graph descriptor returned by AIPU_load_graph
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_GRAPH_NOT_EXIST
 */
aipu_status_t AIPU_unload_graph(const aipu_ctx_handle_t* ctx, const aipu_graph_desc_t* gdesc);
/**
 * @brief This API is used to allocate buffers for all input and output and dump (if any) tensors
 *        of a graph.
 *
 * @param[in]  ctx   Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  gdesc Pointer to a graph descriptor returned by AIPU_load_graph
 * @param[out] info  Pointer to a memory location allocated by application where UMD stores
 *                   the buffer info struct
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_GRAPH_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_BUF_ALLOC_FAIL
 */
aipu_status_t AIPU_alloc_tensor_buffers(const aipu_ctx_handle_t* ctx, const aipu_graph_desc_t* gdesc,
    aipu_buffer_alloc_info_t* info);
/**
 * @brief This API is used to free buffers allocated by AIPU_alloc_tensor_buffers
 *
 * @param[in] ctx    Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] handle Buffer handle returned by AIPU_alloc_tensor_buffers
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_INVALID_HANDLE
 */
aipu_status_t AIPU_free_tensor_buffers(const aipu_ctx_handle_t* ctx, uint32_t handle);
/**
 * @brief This API is used to create a new job for a graph with provided buffer handle.
 *
 * @param[in]  ctx        Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  gdesc      Pointer to a graph descriptor returned by AIPU_load_graph
 * @param[in]  buf_handle Buffer handle returned by AIPU_alloc_tensor_buffers
 * @param[out] job_id     Pointer to a memory location allocated by application where UMD stores
 *                        the new created job ID
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_GRAPH_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_INVALID_HANDLE
 */
aipu_status_t AIPU_create_job(const aipu_ctx_handle_t* ctx, const aipu_graph_desc_t* gdesc,
    uint32_t buf_handle, uint32_t* job_id);
/**
 * @brief This API is used to flush a new computation job onto AIPU
 *
 * @param[in] ctx Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] id  Job ID returned by AIPU_create_job
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 *
 * @note if this API is used in multi-thread applications, the poll_opt flag should be enabled and set
 *       by calling AIPU_set_runtime_config, and also, AIPU_poll_jobs_status should be called in a thread
 *       different from the scheduling thread; in single thread application, application need no such
 *       additional operations.
 */
aipu_status_t AIPU_flush_job(const aipu_ctx_handle_t* ctx, uint32_t id);
/**
 * @brief This API is used to flush a new computation job onto AIPU
 *
 * @param[in] ctx      Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] id       Job ID returned by AIPU_create_job
 * @param[in] time_out Time out (in millisecond) specified by application for this job
 *                     (A timeout of value <= 0 means an infinite timeout.)
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_JOB_EXCEPTION
 * @retval AIPU_STATUS_ERROR_JOB_TIMEOUT
 * @retval Other values returned by AIPU_flush_job
 */
aipu_status_t AIPU_finish_job(const aipu_ctx_handle_t* ctx, uint32_t id, int32_t time_out);
/**
 * @brief This API is used to get the execution status of a job scheduled via AIPU_flush_job.
 *
 * @param[in]  ctx      Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  id       Job ID returned by AIPU_create_job
 * @param[in]  time_out Time out (in millisecond) specified by application for this job
 *                      (A timeout of value <= 0 means an infinite timeout.)
 * @param[out] status   Pointer to a memory location allocated by application where UMD stores
 *                      the job status
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_JOB_NOT_SCHEDULED
 * @retval AIPU_STATUS_ERROR_JOB_TIMEOUT
 */
aipu_status_t AIPU_get_job_status(const aipu_ctx_handle_t* ctx,
    uint32_t id, int32_t time_out, aipu_job_status_t* status);
/**
 * @brief This API is used to poll and update all jobs' status share the same context.
 *        It should be used in a single thread independent with other job scheduling threads.
 *
 * @param[in]  ctx      Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  job_cnt  Pointer to a memory location allocated by application where UMD stores
 *                      the number of jobs state of which are updated in this round of polling
 * @param[in]  time_out Time out (in millisecond) specified by application for polling operation
 *                      (A timeout of value <= 0 means an infinite timeout.)
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 */
aipu_status_t AIPU_poll_jobs_status(const aipu_ctx_handle_t* ctx, uint32_t* job_cnt, int32_t time_out);
/**
 * @brief This API is used to clean a finished job object in UMD
 *
 * @param[in] ctx Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] id  Job ID returned by AIPU_create_job
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_JOB_NOT_SCHEDULED
 */
aipu_status_t AIPU_clean_job(const aipu_ctx_handle_t* ctx, uint32_t id);
/**
 * @brief This API is used to set dump options while debugging
 *
 * @param[in] ctx    Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] job_id Job ID returned by AIPU_create_job
 * @param[in] option Dump option
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_INVALID_OPTIONS
 * @retval AIPU_STATUS_ERROR_INVALID_PATH
 */
aipu_status_t AIPU_set_dump_options(const aipu_ctx_handle_t* ctx, uint32_t job_id, const aipu_dump_option_t* option);
/**
 * @brief This API is used to get number of AIPU core in this system for debugger to use;
 *        The AIPU core IDs are numbered as [0, cnt - 1), respectively;
 *
 * @param[in]  ctx Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[out] cnt Pointer to a memory location allocated by application where UMD stores
 *                 the core count
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 */
aipu_status_t AIPU_debugger_get_core_cnt(const aipu_ctx_handle_t* ctx, uint32_t* cnt);
/**
 * @brief This API is used to get information of an AIPU core for debugger to use
 *
 * @param[in]  ctx  Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[out] info Pointer to a memory location allocated by application where UMD stores
 *                  the core information
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_INVALID_CORE_ID
 * @retval AIPU_STATUS_ERROR_DEV_ABNORMAL
 */
aipu_status_t AIPU_debugger_get_core_info(const aipu_ctx_handle_t* ctx, uint32_t core_id, aipu_core_info_t* info);
/**
 * @brief This API returns physical addresses of corresponding memory sections for debugger to use;
 *        Note that those physical addresses are for host CPU to use, rather than device.
 *
 * @param[in]  ctx    Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in]  job_id Job ID returned by AIPU_create_job
 * @param[out] info   Pointer to a memory location allocated by application where UMD stores
 *                    the debugging info of a job
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 */
aipu_status_t AIPU_debugger_get_job_info(const aipu_ctx_handle_t* ctx, uint32_t job_id, aipu_debugger_job_info_t* info);
/**
 * @brief This API bind a created job to an idle AIPU core for execution later;
 *        External registers of the specified AIPU core is writen after this API returns,
 *        but the start PC register is not triggerred to run.
 *
 * @param[in] ctx     Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] job_id  Job ID returned by AIPU_create_job
 * @param[in] core_id AIPU core ID
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_INVALID_CORE_ID
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_INVALID_OP
 */
aipu_status_t AIPU_debugger_bind_job(const aipu_ctx_handle_t* ctx, uint32_t core_id, uint32_t job_id);
/**
 * @brief This API trigger a previously bind job to run on a target AIPU core;
 *        This API is a blocking API which returns after the job execution ends on hardware.
 *
 * @param[in] ctx     Pointer to a context handle struct returned by AIPU_init_ctx
 * @param[in] job_id  Job ID returned by AIPU_create_job and bind by AIPU_debugger_bind_job
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 * @retval AIPU_STATUS_ERROR_INVALID_CTX
 * @retval AIPU_STATUS_ERROR_JOB_NOT_EXIST
 * @retval AIPU_STATUS_ERROR_JOB_EXCEPTION
 * @retval AIPU_STATUS_ERROR_INVALID_OP
 */
aipu_status_t AIPU_debugger_run_job(const aipu_ctx_handle_t* ctx, uint32_t job_id);
/**
 * @brief this API print AIPU execution log information after corresponding job ends
 *
 * @param[in] printf_dumps  Pointer to a tensor buffer allocated by UMD stores the log information
 * @param[in] redirect_file Printf output redirect file path
 *
 * @retval AIPU_STATUS_SUCCESS
 * @retval AIPU_STATUS_ERROR_NULL_PTR
 *
 */
aipu_status_t AIPU_printf(aipu_tensor_buffer_t* printf_dumps, char* redirect_file);

#endif /* _STANDARD_API_H_ */
