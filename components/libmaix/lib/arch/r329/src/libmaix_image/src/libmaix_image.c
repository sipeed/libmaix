
#include "stdlib.h"
#include "libmaix_debug.h"
#include "libmaix_image.h"

libmaix_err_t libmaix_image_module_init()
{
    // return libmaix_image_hal_module_init();
}

libmaix_err_t libmaix_image_module_deinit()
{
    // return libmaix_image_hal_module_deinit();
}

libmaix_image_t* libmaix_image_create(int w, int h, libmaix_image_mode_t mode, libmaix_image_layout_t layout, void* data, bool is_data_alloc)
{
    uint64_t img_size = 0;

    if( !(mode==LIBMAIX_IMAGE_MODE_RGB888 || mode==LIBMAIX_IMAGE_MODE_YUV420SP_NV21) )
    {
        LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_PARAM);
        return NULL;
    }
    if( !(w==0 || h==0 || mode==LIBMAIX_IMAGE_MODE_INVALID) )
    { 
        if(!data)
        {
            switch(mode)
            {
                case LIBMAIX_IMAGE_MODE_RGB888:
                    img_size = w * h * 3;
                    break;
                case LIBMAIX_IMAGE_MODE_YUV420SP_NV21:
                    img_size = w * h * 3 / 2;
                    break;
                default:
                    LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_PARAM);
                    return NULL;
            }
            data = malloc(img_size);
            if(!data)
            {
                LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NO_MEM);
                return NULL;
            }
            is_data_alloc = true;
        }
    }
    libmaix_image_t* img = (libmaix_image_t*)malloc(sizeof(libmaix_image_t));
    if(!img)
    {
        LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NO_MEM);
        return NULL;
    }
    img->width  = w;
    img->height = h;
    img->mode   = mode;
    img->layout = layout;
    img->data   = data;
    img->is_data_alloc = is_data_alloc;

    // img->convert        = libmaix_image_hal_convert;
    // img->draw_rectangle = libmaix_image_hal_draw_rectangle;
    // img->draw_string    = libmaix_image_hal_draw_string;
    // img->resize         = libmaix_image_hal_resize;
    // img->crop           = libmaix_image_hal_crop;
    return img;
}

void libmaix_image_destroy(libmaix_image_t **obj)
{
    if(*obj)
    {
        if((*obj)->is_data_alloc)
        {
            free((*obj)->data);
            (*obj)->data = NULL;
        }
        free(*obj);
        *obj = NULL;
    }
}

