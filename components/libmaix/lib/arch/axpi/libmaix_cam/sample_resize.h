#pragma once

#include "npu_cv_kit/npu_common.h"
#include "ax_interpreter_external_api.h"

class ax_crop_resize_nv12
{
public:
    ax_crop_resize_nv12() = default;
    ~ax_crop_resize_nv12();
    int init(int input_h, int input_w, int model_h, int model_w, int model_type);
    int run_crop_resize_nv12(void* input_data);
    int run_crop_resize_nv12(void* input_data, void* output_data);

    AX_NPU_CV_Image* m_input_image = nullptr;
    AX_NPU_CV_Image* m_output_image = nullptr;
    AX_NPU_CV_Box* m_box = nullptr;

    int m_model_type = AX_NPU_MODEL_TYPE_1_1_1;
};