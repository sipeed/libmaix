
#include "libmaix_image.h"
#include "libmaix_debug.h"
#include <cstdlib>

// inline static unsigned short make16color(unsigned char r, unsigned char g, unsigned char b)
// {
// 	return (
// 	(((r >> 3) & 31) << 11) |
// 	(((g >> 2) & 63) << 5)  |
// 	 ((b >> 3) & 31)		);
// }

libmaix_err_t libmaix_image_soft_convert(struct libmaix_image *obj, libmaix_image_mode_t mode, struct libmaix_image **new_img)
{

	libmaix_err_t err = LIBMAIX_ERR_NONE;
	if(new_img == NULL)
	{
		return LIBMAIX_ERR_PARAM;
	}
	if(mode == obj->mode)
	{
		return LIBMAIX_ERR_NONE;
	}
	if(obj->width==0 || obj->height==0 || obj->data == NULL)
	{
		return LIBMAIX_ERR_PARAM;
	}

  int new_mem = 0;
  if((*new_img) == NULL)
  {
    *new_img = libmaix_image_create(obj->width, obj->height, obj->mode, obj->layout, NULL, true);
    if(!(*new_img))
    {
      return LIBMAIX_ERR_NO_MEM;
    }
    new_mem = 1;
  }
  else
  {
    if( (*new_img)->data == NULL)
    {
      (*new_img)->data = malloc(obj->width * obj->height * 3);
      if(!(*new_img)->data)
      {
        return LIBMAIX_ERR_NO_MEM;
      }
      (*new_img)->is_data_alloc = true;
      new_mem = 2;
    }
    (*new_img)->layout = obj->layout;
    (*new_img)->width = obj->width;
    (*new_img)->height = obj->height;
  }

  // -------------------------------
  switch(obj->mode)
  {
    case LIBMAIX_IMAGE_MODE_RGB888: {
      switch (mode)
      {
        case LIBMAIX_IMAGE_MODE_RGB565: {
          if (obj == *new_img || obj->width != (*new_img)->width || obj->height != (*new_img)->height) return LIBMAIX_ERR_PARAM;
          uint8_t *rgb888 = (uint8_t *)obj->data;
          uint16_t *rgb565 = (uint16_t *)(*new_img)->data;
          for (uint16_t *end = rgb565 + obj->width * obj->height; rgb565 < end; rgb565 += 1, rgb888 += 3) {
            // *rgb565 = make16color(rgb888[0], rgb888[1], rgb888[2]);
            *rgb565 = ((((rgb888[0] >> 3) & 31) << 11) | (((rgb888[1] >> 2) & 63) << 5) | ((rgb888[2] >> 3) & 31));
          }
          (*new_img)->mode = mode;
          break;
        }
        case LIBMAIX_IMAGE_MODE_BGR888: {
          // printf("libmaix_image_hal_convert obj->mode %d mode %d \r\n", obj->mode, mode);
          uint8_t *rgb = (uint8_t *)(obj->data), *bgr = (uint8_t *)(*new_img)->data;
          for (uint8_t *end = rgb + obj->width * obj->height * 3; rgb < end; rgb += 3, bgr += 3) {
            bgr[2] = rgb[0], bgr[1] = rgb[1], bgr[0] = rgb[2];
          }
          (*new_img)->mode = mode;
          break;
        }
        default:
          err = LIBMAIX_ERR_PARAM;
          break;
      }
      break;
    }
    default:
      err = LIBMAIX_ERR_NOT_IMPLEMENT;
      break;
  }
  // -------------------------------

  if(err != LIBMAIX_ERR_NONE)
  {
    if(new_mem == 2)
    {
      free((*new_img)->data);
      (*new_img)->data = NULL;
    }
    else if (new_mem == 1)
    {
      libmaix_image_destroy(new_img);
    }
  }
	return err;
}

libmaix_err_t libmaix_image_soft_resize(struct libmaix_image *obj, int w, int h, struct libmaix_image **new_img)
{
  LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
  return LIBMAIX_ERR_NOT_IMPLEMENT;
}

libmaix_err_t libmaix_image_soft_crop(struct libmaix_image *obj, int x, int y, int w, int h, struct libmaix_image **new_img)
{
  LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
  return LIBMAIX_ERR_NOT_IMPLEMENT;
}

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdlib.h"

  libmaix_err_t libmaix_image_module_init()
  {
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_image_module_deinit()
  {
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_image_t *libmaix_image_create(int w, int h, libmaix_image_mode_t mode, libmaix_image_layout_t layout, void *data, bool is_data_alloc)
  {
    uint64_t img_size = 0;

    if( !(mode==LIBMAIX_IMAGE_MODE_RGB565 ||
          mode==LIBMAIX_IMAGE_MODE_RGB888 ||
          mode==LIBMAIX_IMAGE_MODE_BGR888 ||
          mode==LIBMAIX_IMAGE_MODE_RGBA8888 ||
          mode==LIBMAIX_IMAGE_MODE_YUV420SP_NV21 ||
          mode==LIBMAIX_IMAGE_MODE_YUV422_YUYV ||
          mode==LIBMAIX_IMAGE_MODE_GRAY) )
    {
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_PARAM);
      return NULL;
    }
    if( !(w==0 || h==0 || mode==LIBMAIX_IMAGE_MODE_INVALID || !is_data_alloc))
    {
      if (!data)
      {
        switch(mode)
        {
            case LIBMAIX_IMAGE_MODE_RGBA8888:
                img_size = w * h * 4;
                break;
            case LIBMAIX_IMAGE_MODE_BGR888:
            case LIBMAIX_IMAGE_MODE_RGB888:
                img_size = w * h * 3;
                break;
            case LIBMAIX_IMAGE_MODE_RGB565:
            case LIBMAIX_IMAGE_MODE_YUV422_YUYV:
                img_size = w * h * 2;
                break;
            case LIBMAIX_IMAGE_MODE_YUV420SP_NV21:
                img_size = w * h * 3 / 2;
                break;
            case LIBMAIX_IMAGE_MODE_GRAY:
                img_size = w * h;
                break;
            default:
                LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_PARAM);
                return NULL;
        }
        data = malloc(img_size);
        if (!data)
        {
          LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NO_MEM);
          return NULL;
        }
        is_data_alloc = true;
      }
    }
    libmaix_image_t *img = (libmaix_image_t *)malloc(sizeof(libmaix_image_t));
    if (!img)
    {
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NO_MEM);
      return NULL;
    }
    img->width = w;
    img->height = h;
    img->mode = mode;
    img->layout = layout;
    img->data = data;
    img->is_data_alloc = is_data_alloc;

    img->convert = libmaix_image_soft_convert;
    // img->draw_rectangle = libmaix_image_soft_draw_rectangle;
    // img->draw_string    = libmaix_image_soft_draw_string;
    img->resize = libmaix_image_soft_resize;
    img->crop = libmaix_image_soft_crop;
    return img;
  }

  void libmaix_image_destroy(libmaix_image_t **obj)
  {
    if(NULL == obj || NULL == *obj)
        return;

    if ((*obj)->is_data_alloc)
    {
      free((*obj)->data);
      (*obj)->data = NULL;
    }
    free(*obj);
    *obj = NULL;

  }

#ifdef __cplusplus
}
#endif
