#ifndef __LIBMAIX_NN_CLASSIFIER_H
#define __LIBMAIX_NN_CLASSIFIER_H

#include "libmaix_nn.h"
#include "libmaix_image.h"
#include "stdint.h"
#include "stdbool.h"

libmaix_err_t libmaix_classifier_init(void** obj, libmaix_nn_t* model, int feature_length, int input_w, int input_h, int class_num, int sample_num);
/**
 *
 * @param[in/out] idx add class img as idx class, if idx < 0, will automatically add as next class, and set this value as set idx
 */
libmaix_err_t libmaix_classifier_add_class_img(void* obj, libmaix_image_t* img, int* idx);
libmaix_err_t libmaix_classifier_rm_class_img(void* obj);
/**
 *
 * @param[out] idx fixed pass -1, and return added sample img feature index
 */
libmaix_err_t libmaix_classifier_add_sample_img(void* obj, libmaix_image_t* img, int* idx);
libmaix_err_t libmaix_classifier_rm_sample_img(void* obj);
libmaix_err_t libmaix_classifier_del(void** obj);
libmaix_err_t libmaix_classifier_train(void* obj);
libmaix_err_t libmaix_classifier_predict(void* obj, libmaix_image_t* img, int* idx, float* min_distance);
libmaix_err_t libmaix_classifier_save(void* obj, const char* path);
libmaix_err_t libmaix_classifier_load(void** obj, const char* path, libmaix_nn_t* kmodel, int* feature_length, int* input_w, int* input_h, int* class_num, int* sample_num);

#endif