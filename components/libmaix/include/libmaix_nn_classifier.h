#ifndef __LIBMAIX_NN_CLASSIFIER_H
#define __LIBMAIX_NN_CLASSIFIER_H

#include "libmaix_nn.h"
#include "libmaix_image.h"
#include "stdint.h"
#include "stdbool.h"

int libmaix_classifier_init(void** obj, libmaix_nn_t* model, int samples_length, int input_w, int input_h, int class_num, int sample_num);
int libmaix_classifier_add_class_img(void* obj, libmaix_image_t* img, int idx);
int libmaix_classifier_rm_class_img(void* obj);
int libmaix_classifier_add_sample_img(void* obj, libmaix_image_t* img);
int libmaix_classifier_rm_sample_img(void* obj);
int libmaix_classifier_del(void** obj);
int libmaix_classifier_train(void* obj);
int libmaix_classifier_predict(void* obj, libmaix_image_t* img, float* min_distance);



#endif
