/*
 * AXERA is pleased to support the open source community by making ax-samples available.
 *
 * Copyright (c) 2022, AXERA Semiconductor (Shanghai) Co., Ltd. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

 /*
  * Author: hc.zhao
  */

#include <cstdio>
#include <cstring>
#include <numeric>

#include <opencv2/opencv.hpp>

#include "base/topk.hpp"
#include "base/detection.hpp"
#include "base/common.hpp"
#include "middleware/io.hpp"

#include "utilities/args.hpp"
#include "utilities/cmdline.hpp"
#include "utilities/file.hpp"
#include "utilities/timer.hpp"

#include "ax_interpreter_external_api.h"
#include "ax_sys_api.h"
#include "joint.h"
#include "joint_adv.h"

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "libmaix_cam.h"
#include "libmaix_image.h"
#include "libmaix_disp.h"
#include "libmaix_cv_image.h"
#include "main.h"
#include <signal.h>

const int DEFAULT_IMG_H = 640;
const int DEFAULT_IMG_W = 640;
const char* CLASS_NAMES[] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush" };
const float ANCHORS[18] = { 10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119, 116, 90, 156, 198, 373, 326 };

const int DEFAULT_LOOP_COUNT = 1;

const float PROB_THRESHOLD = 0.35f;
const float NMS_THRESHOLD = 0.45f;



struct {
    int w0, h0;
    struct libmaix_cam* cam0;
    struct libmaix_disp* disp;
    int is_run;
} app = { 0 };

int min(int a, int b)
{
    return a < b ? a : b;
}


bool get_image(std::vector<uint8_t>& data, int w, int h, libmaix_image_t** img)
{
    uint8_t* data_p = data.data();
    libmaix_image_t small_img;
    small_img.data = data_p;
    small_img.width = w;
    small_img.height = h;
    small_img.mode = LIBMAIX_IMAGE_MODE_RGB888;
    small_img.layout = LIBMAIX_IMAGE_LAYOUT_HWC;
    libmaix_image_t* small_img_p = &small_img;
    timer get_cam_timer;
    if (LIBMAIX_ERR_NONE == app.cam0->capture_image(app.cam0, img))
    {    
        uint32_t get_cam_timer_us = (uint32_t)(get_cam_timer.cost() * 1000);
        fprintf(stdout, "time cost of get cam—img : %ld\n",get_cam_timer_us / 1000);
        timer resize_timer;
        libmaix_cv_image_resize(*img, w, h, &small_img_p);
        uint32_t resize_timer_us = (uint32_t)(resize_timer.cost() * 1000);
        fprintf(stdout, "time cost of resize image : %ld\n",resize_timer_us / 1000);
        return true;
    }
    else
    {
        printf("!!! capture_image failed\r\n");
        return false;
    }

}

namespace ax
{
    namespace det = detection;
    namespace mw = middleware;
    namespace utl = utilities;
    
    bool run_detection(const std::string& model, int input_w, int input_h)
    {
        int repeat = 1;

        // 1. create a runtime handle and load the model
        AX_JOINT_HANDLE joint_handle;
        std::memset(&joint_handle, 0, sizeof(joint_handle));

        AX_JOINT_SDK_ATTR_T joint_attr;
        std::memset(&joint_attr, 0, sizeof(joint_attr));

        // 1.1 read model file to buffer
        std::vector<char> model_buffer;
        if (!ax::utl::read_file(model, model_buffer))
        {
            fprintf(stderr, "Read Run-Joint model(%s) file failed.\n", model.c_str());
            return false;
        }

        // 1.2 parse model from buffer
        //   if the device do not have enough memory to create a buffer at step 3.1,
        //     consider using linux API 'mmap' to map the model file to a pointer,
        //     then use the pointer which returned by mmap to parse the run-joint
        //     model from the file.
        //   it will reduce the peak allocated memory(compared with creating a full
        //     size buffer).
        auto ret = ax::mw::parse_npu_mode_from_joint(model_buffer.data(), model_buffer.size(), &joint_attr.eNpuMode);
        if (AX_ERR_NPU_JOINT_SUCCESS != ret)
        {
            fprintf(stderr, "Load Run-Joint model(%s) failed.\n", model.c_str());
            return false;
        }
        
        // 1.3 init model
        ret = AX_JOINT_Adv_Init(&joint_attr);
        if (AX_ERR_NPU_JOINT_SUCCESS != ret)
        {
            fprintf(stderr, "Init Run-Joint model(%s) failed.\n", model.c_str());
            return false;
        }

        auto deinit_joint = [&joint_handle]() {
            AX_JOINT_DestroyHandle(joint_handle);
            AX_JOINT_Adv_Deinit();
            return false;
        };
        
        // 1.4 the real init processing
        uint32_t duration_hdl_init_us = 0;
        {
            timer init_timer;
            ret = AX_JOINT_CreateHandle(&joint_handle, model_buffer.data(), model_buffer.size());
            duration_hdl_init_us = (uint32_t)(init_timer.cost() * 1000);
            if (AX_ERR_NPU_JOINT_SUCCESS != ret)
            {
                fprintf(stderr, "Create Run-Joint handler from file(%s) failed.\n", model.c_str());
                return deinit_joint();
            }
        }
        
        // 1.5 get the version of toolkit (optional)
        const AX_CHAR* version = AX_JOINT_GetModelToolsVersion(joint_handle);
        fprintf(stdout, "Tools version: %s\n", version);

        // 1.6 drop the model buffer
        std::vector<char>().swap(model_buffer);
        auto io_info = AX_JOINT_GetIOInfo(joint_handle);

        // 1.7 create context
        AX_JOINT_EXECUTION_CONTEXT joint_ctx;
        AX_JOINT_EXECUTION_CONTEXT_SETTING_T joint_ctx_settings;
        std::memset(&joint_ctx, 0, sizeof(joint_ctx));
        std::memset(&joint_ctx_settings, 0, sizeof(joint_ctx_settings));
        ret = AX_JOINT_CreateExecutionContextV2(joint_handle, &joint_ctx, &joint_ctx_settings);
        if (AX_ERR_NPU_JOINT_SUCCESS != ret)
        {
            fprintf(stderr, "Create Run-Joint context failed.\n");
            return deinit_joint();
        }

        // 2. fill input & prepare to inference
        AX_JOINT_IO_T joint_io_arr;
        AX_JOINT_IO_SETTING_T joint_io_setting; 
        auto clear_and_exit = [&joint_io_arr, &joint_ctx, &joint_handle]() {
            for (size_t i = 0; i < joint_io_arr.nInputSize; ++i)
            {
                AX_JOINT_IO_BUFFER_T* pBuf = joint_io_arr.pInputs + i;
                mw::free_joint_buffer(pBuf);
            }
            for (size_t i = 0; i < joint_io_arr.nOutputSize; ++i)
            {
                AX_JOINT_IO_BUFFER_T* pBuf = joint_io_arr.pOutputs + i;
                mw::free_joint_buffer(pBuf);
            }
            delete[] joint_io_arr.pInputs;
            delete[] joint_io_arr.pOutputs;

            AX_JOINT_DestroyExecutionContext(joint_ctx);
            AX_JOINT_DestroyHandle(joint_handle);
            AX_JOINT_Adv_Deinit();

            return false;
        };


        // loop to get image from camera and do inference
        libmaix_image_t* img = NULL;
        std::vector<uint8_t> data(input_w * input_h * 3);
        while (app.is_run)
        {
            timer get_img_timer;
            // get image from camera
            bool ok = get_image(data, input_w, input_h, &img);
            uint32_t get_img_timer_us = (uint32_t)(get_img_timer.cost() * 1000);
            fprintf(stdout, "time cost of get image : %ld\n",get_img_timer_us / 1000);
            timer get_img1_timer;
            
            if (!ok)
            {
                continue;
            }

            // 2. fill input & prepare to inference
            std::memset(&joint_io_arr, 0, sizeof(joint_io_arr));
            std::memset(&joint_io_setting, 0, sizeof(joint_io_setting));
            ret = mw::prepare_io(data.data(), data.size(), joint_io_arr, io_info);
            if (AX_ERR_NPU_JOINT_SUCCESS != ret)
            {
                fprintf(stderr, "Fill input failed.\n");
                AX_JOINT_DestroyExecutionContext(joint_ctx);
                return deinit_joint();
            }
            joint_io_arr.pIoSetting = &joint_io_setting;

            fprintf(stdout, "3. init\n");
            // 3. get the init profile info.
            AX_JOINT_COMPONENT_T* joint_comps;
            uint32_t joint_comp_size;

            ret = AX_JOINT_ADV_GetComponents(joint_ctx, &joint_comps, &joint_comp_size);
            if (AX_ERR_NPU_JOINT_SUCCESS != ret)
            {
                fprintf(stderr, "Get components failed.\n");
                return clear_and_exit();
            }

            uint32_t duration_neu_init_us = 0;
            uint32_t duration_axe_init_us = 0;
            for (uint32_t j = 0; j < joint_comp_size; ++j)
            {
                auto& comp = joint_comps[j];
                switch (comp.eType)
                {
                case AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_NEU:
                {
                    duration_neu_init_us += comp.tProfile.nInitUs;
                    break;
                }
                case AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_AXE:
                {
                    duration_axe_init_us += comp.tProfile.nInitUs;
                    break;
                }
                default:
                    fprintf(stderr, "Unknown component type %d.\n", (int)comp.eType);
                }
            }
            
            uint32_t step2_3_us = (uint32_t)(get_img1_timer.cost() * 1000);
            fprintf(stdout, "time cost of step2-3 : %ld\n",step2_3_us /1000);
            timer get_img2_timer;
            
            fprintf(stdout, "4.run & benchmark\n");
            // 4. run & benchmark
            uint32_t duration_neu_core_us = 0, duration_neu_total_us = 0;
            uint32_t duration_axe_core_us = 0, duration_axe_total_us = 0;

            std::vector<float> time_costs(repeat, 0.f);
            for (int i = 0; i < repeat; ++i)
            {
                timer tick;
                ret = AX_JOINT_RunSync(joint_handle, joint_ctx, &joint_io_arr);
                time_costs[i] = tick.cost();
                if (AX_ERR_NPU_JOINT_SUCCESS != ret)
                {
                    fprintf(stderr, "Inference failed(%d).\n", ret);
                    return clear_and_exit();
                }

                ret = AX_JOINT_ADV_GetComponents(joint_ctx, &joint_comps, &joint_comp_size);
                if (AX_ERR_NPU_JOINT_SUCCESS != ret)
                {
                    fprintf(stderr, "Get components after run failed.\n");
                    return clear_and_exit();
                }

                for (uint32_t j = 0; j < joint_comp_size; ++j)
                {
                    auto& comp = joint_comps[j];

                    if (comp.eType == AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_NEU)
                    {
                        duration_neu_core_us += comp.tProfile.nCoreUs;
                        duration_neu_total_us += comp.tProfile.nTotalUs;
                    }

                    if (comp.eType == AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_AXE)
                    {
                        duration_axe_core_us += comp.tProfile.nCoreUs;
                        duration_axe_total_us += comp.tProfile.nTotalUs;
                    }
                }
            }
            fprintf(stdout, "run over: output len %d\n", io_info->nOutputSize);
            
            uint32_t run_us = (uint32_t)(get_img2_timer.cost() * 1000);
            fprintf(stdout, "time cost of run : %ld\n", run_us /1000);
            timer get_img3_timer;
            
            fprintf(stdout, "5.get bbox\n");
            // 5. get bbox
            std::vector<det::Object> proposals;
            std::vector<det::Object> objects;

            float prob_threshold_unsigmoid = -1.0f * (float)std::log((1.0f / PROB_THRESHOLD) - 1.0f);
            for (uint32_t i = 0; i < io_info->nOutputSize; ++i)
            {
                auto& output = io_info->pOutputs[i];
                auto& info = joint_io_arr.pOutputs[i];

                auto ptr = (float*)info.pVirAddr;

                int32_t stride = (1 << i) * 8;
                
                //下面的函数在ax-samples/examples/base/detection.hpp中，该函数实现了对预测框的解码 类别数改变需要改这个函数
                det::generate_proposals_255(stride, ptr, PROB_THRESHOLD, proposals, input_w, input_h, ANCHORS, prob_threshold_unsigmoid);
            }
            uint32_t prop_us = (uint32_t)(get_img3_timer.cost() * 1000);
            fprintf(stdout, "time cost of generate_prop : %ld\n", prop_us/1000);
            timer get_img4_timer;
            
            //包含nms等，实现bbox的输出
            det::get_out_bbox_no_letterbox(proposals, objects, NMS_THRESHOLD, input_h, input_w, img->height, img->width);
            
            uint32_t outbbox_us = (uint32_t)(get_img4_timer.cost() * 1000);
            fprintf(stdout, "time cost of outbbox : %ld\n", outbbox_us/1000);
            timer get_img5_timer;
            
            fprintf(stdout, "object size %d\n",  objects.size());
            for (size_t i = 0; i < objects.size(); i++)
            {
                const det::Object& obj = objects[i];

                fprintf(stdout, "%2d: %3.0f%%, [%4.0f, %4.0f, %4.0f, %4.0f], %s\n", obj.label, obj.prob * 100, obj.rect.x,
                    obj.rect.y, obj.rect.x + obj.rect.width, obj.rect.y + obj.rect.height, CLASS_NAMES[obj.label]);

                //调用已经实现的api画框
                char buf[64];
                snprintf(buf, sizeof(buf), "%3.0f%% : %s", obj.prob * 100, CLASS_NAMES[obj.label]);

                libmaix_image_color_t color = { 255, 0, 0, 255 };
                libmaix_cv_image_draw_rectangle(img, obj.rect.x,
                    obj.rect.y, obj.rect.x + obj.rect.width, obj.rect.y + obj.rect.height, color, 1);
                libmaix_cv_image_draw_string(img, obj.rect.x,
                    obj.rect.y, buf, 1.0, color, 2);
            }
            
            uint32_t bbox_us = (uint32_t)(get_img5_timer.cost() * 1000);
            fprintf(stdout, "time cost of bbox : %ld\n", bbox_us/1000);
            timer get_img6_timer;
            
            app.disp->draw_image(app.disp, img);
            
            for (size_t i = 0; i < joint_io_arr.nInputSize; ++i)
                {
                    AX_JOINT_IO_BUFFER_T* pBuf = joint_io_arr.pInputs + i;
                    mw::free_joint_buffer(pBuf);
                }
            for (size_t i = 0; i < joint_io_arr.nOutputSize; ++i)
                {
                    AX_JOINT_IO_BUFFER_T* pBuf = joint_io_arr.pOutputs + i;
                    mw::free_joint_buffer(pBuf);
                }
            delete[] joint_io_arr.pInputs;
            delete[] joint_io_arr.pOutputs;
            
            uint32_t del_us = (uint32_t)(get_img6_timer.cost() * 1000);
            fprintf(stdout, "time cost of delete IObuffer : %ld\n", del_us /1000);
        }
        // destroy context
        libmaix_image_destroy(&img);

        clear_and_exit();
        return true;
    }
}

static void app_handle_signal(int signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
    {
        app.is_run = 0;
    }
}

void cam_init()
{
    signal(SIGINT, app_handle_signal);
    signal(SIGTERM, app_handle_signal);

    libmaix_image_module_init();
    libmaix_camera_module_init();
    app.w0 = 854, app.h0 = 480;

    app.cam0 = libmaix_cam_create(0, app.w0, app.h0, 1, 0);
    if (NULL == app.cam0) return;

    app.disp = libmaix_disp_create(0);

    if (NULL == app.disp) return;
    if (app.disp->width == 0 || app.disp->height == 0) app.disp->width = app.w0, app.disp->height = app.h0;

    app.is_run = 1;
    app.cam0->start_capture(app.cam0);
}

void cam_deinit()
{
    if (NULL != app.cam0) libmaix_cam_destroy(&app.cam0);
    if (NULL != app.disp) libmaix_disp_destroy(&app.disp), app.disp = NULL;

    libmaix_camera_module_deinit();
    libmaix_image_module_deinit();
}

int main(int argc, char* argv[])
{

    cmdline::parser cmd;
    cmd.add<std::string>("model", 'm', "joint file(a.k.a. joint model)", true, "");
    cmd.add<std::string>("size", 'g', "input_h, input_w", false, std::to_string(DEFAULT_IMG_H) + "," + std::to_string(DEFAULT_IMG_W));

    cmd.parse_check(argc, argv);

    // 0. get app args, can be removed from user's app
    auto model_file = cmd.get<std::string>("model");

    auto model_file_flag = utilities::file_exist(model_file);

    if (!model_file_flag)
    {
        auto show_error = [](const std::string& kind, const std::string& value)
        {
            fprintf(stderr, "Input file %s(%s) is not exist, please check it.\n", kind.c_str(), value.c_str());
        };

        if (!model_file_flag) { show_error("model", model_file); }

        return -1;
    }

    auto input_size_string = cmd.get<std::string>("size");

    std::array<int, 2> input_size = { DEFAULT_IMG_H, DEFAULT_IMG_W };

    auto input_size_flag = utilities::parse_string(input_size_string, input_size);

    if (!input_size_flag)
    {
        auto show_error = [](const std::string& kind, const std::string& value)
        {
            fprintf(stderr, "Input %s(%s) is not allowed, please check it.\n", kind.c_str(), value.c_str());
        };

        if (!input_size_flag) { show_error("size", input_size_string); }

        return -1;
    }

    // 1. print args
    fprintf(stdout, "--------------------------------------\n");

    fprintf(stdout, "model file : %s\n", model_file.c_str());
    fprintf(stdout, "img_h, img_w : %d %d\n", input_size[0], input_size[1]);

    cam_init();


    // 4. show the version (optional)
    fprintf(stdout, "Run-Joint Runtime version: %s\n", AX_JOINT_GetVersion());
    fprintf(stdout, "----------------------------\n");

    // 5. run the processing 
    auto flag = ax::run_detection(model_file, input_size[0], input_size[1]);

    if (!flag)
    {
        fprintf(stderr, "Run detection failed.\n");
    }
    cam_deinit();
}

