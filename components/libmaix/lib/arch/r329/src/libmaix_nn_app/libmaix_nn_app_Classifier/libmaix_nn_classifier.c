#include "libmaix_nn_classifier.h"

libmaix_err_t libmaix_classifier_init(void** obj, libmaix_nn_t* model, int feature_length, int input_w, int input_h, int class_num, int sample_num)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_add_class_img(void* obj, libmaix_image_t* img, int* idx)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_rm_class_img(void* obj)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_add_sample_img(void* obj, libmaix_image_t* img, int* idx)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_rm_sample_img(void* obj)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_del(void** obj)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_train(void* obj)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_predict(void* obj, libmaix_image_t* img, int* idx, float* min_distance)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_save(void* obj, const char* path)
{
    return LIBMAIX_ERR_NONE;
}
libmaix_err_t libmaix_classifier_load(void** obj, const char* path, libmaix_nn_t* kmodel, int* feature_length, int* input_w, int* input_h, int* class_num, int* sample_num)
{
    return LIBMAIX_ERR_NONE;
}



