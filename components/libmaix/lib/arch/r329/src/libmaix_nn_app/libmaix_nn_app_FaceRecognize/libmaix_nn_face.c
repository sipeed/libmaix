#include "libmaix_nn_face.h"

libmaix_err_t libmaix_nn_face_recognize_init(void** obj, libmaix_nn_face_config_t* config, libmaix_nn_t* model_detect, libmaix_nn_t* model_landmark, libmaix_nn_t* model_feature, int fea_len, int max_face_num, libmaix_nn_face_small2big_func_t map_func, void* map_func_args)
{    
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_face_recognize_deinit(void** obj)
{
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_nn_face_get_feature(void* obj, libmaix_image_t* img, int* face_num, libmaix_image_t* img_big, face_obj_t** results, bool get_std_img)
{
    return LIBMAIX_ERR_NONE;
}
