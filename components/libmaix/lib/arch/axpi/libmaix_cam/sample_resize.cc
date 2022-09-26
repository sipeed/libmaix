#include "sample_resize.h"
#include "cv/cv.hpp"

int ax_crop_resize_nv12::init(int input_h, int input_w, int model_h, int model_w, int model_type)
{
    axcv::ax_image input;
    input.w = input_w;
    input.h = input_h;
    input.stride_w = input.w;
    input.color_space = AX_NPU_CV_FDT_NV12;
    m_input_image = axcv::alloc_cv_image(input);
    if (!m_input_image)
    {
        fprintf(stderr, "[ERR] alloc_cv_image input falil \n");
        return -1;
    }

    axcv::ax_box box;
    box.h = input_h;
    box.w = input_w;
    box.x = 0;
    box.y = 0;
    m_box = axcv::filter_box(input, box);
    if (!m_box)
    {
        fprintf(stderr, "[ERR] box not legal \n");
        return -1;
    }

    axcv::ax_image output;
    output.w = model_w;
    output.h = model_h;
    output.stride_w = output.w;
    output.color_space = AX_NPU_CV_FDT_NV12;
    m_output_image = axcv::alloc_cv_image(output);
    if (!m_output_image)
    {
        fprintf(stderr, "[ERR] alloc_cv_image output falil \n");
        return -1;
    }

    m_model_type = model_type;

    return 0;
}

int ax_crop_resize_nv12::run_crop_resize_nv12(void* input_data)
{
    int ret = axcv::npu_crop_resize(m_input_image, (const char*)input_data, m_output_image, m_box, (AX_NPU_SDK_EX_MODEL_TYPE_T)m_model_type);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS)
    {
        fprintf(stderr, "[ERR] npu_crop_resize err code:%x \n", ret);
        return ret;
    }
    return AX_NPU_CV_SUCCESS;
}

int ax_crop_resize_nv12::run_crop_resize_nv12(void* input_data, void* output_data)
{
    int ret = axcv::npu_crop_resize(m_input_image, (const char*)input_data, m_output_image, m_box, (AX_NPU_SDK_EX_MODEL_TYPE_T)m_model_type);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS)
    {
        fprintf(stderr, "[ERR] npu_crop_resize err code:%x \n", ret);
        return ret;
    }

    memcpy(output_data, m_output_image->pVir, m_output_image->tStride.nW * m_output_image->nHeight * 3 / 2);

    return AX_NPU_CV_SUCCESS;
}

ax_crop_resize_nv12::~ax_crop_resize_nv12()
{
    axcv::free_cv_image(m_input_image);
    axcv::free_cv_image(m_output_image);
    delete m_box;
}