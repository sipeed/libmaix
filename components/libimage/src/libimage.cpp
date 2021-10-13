
class libimage
{
private:
  /* data */
public:
  libimage(/* args */);
  ~libimage();
};

libimage::libimage(/* args */)
{

}

libimage::~libimage()
{

};

extern "C" {
  #include "libimage.h"
  #include "stdlib.h"
  #include "libmaix_debug.h"


  libmaix_err_t libtest_image_module_init()
  {
      // return libtest_image_hal_module_init();
  }

  libmaix_err_t libtest_image_module_deinit()
  {
      // return libtest_image_hal_module_deinit();
  }

  libtest_image_t* libtest_image_create(int w, int h, libtest_image_mode_t mode, libtest_image_layout_t layout, void* data, bool is_data_alloc)
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
      libtest_image_t* img = (libtest_image_t*)malloc(sizeof(libtest_image_t));
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

      // img->convert        = libtest_image_hal_convert;
      // img->draw_rectangle = libtest_image_hal_draw_rectangle;
      // img->draw_string    = libtest_image_hal_draw_string;
      // img->resize         = libtest_image_hal_resize;
      // img->crop           = libtest_image_hal_crop;
      return img;
  }

  void libtest_image_destroy(libtest_image_t **obj)
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


  #include "stdio.h"

  void libimage_cpp()
  {
    auto t = libimage();

    puts("libimage_cpp");
    
    libtest_image_module_init();

    libtest_image_module_deinit();

    printf("--create image\n");
    libtest_image_t *img = libtest_image_create(240, 240, LIBMAIX_IMAGE_MODE_YUV420SP_NV21, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    if(!img)
    {
        printf("create yuv image fail\n");
    }
    libtest_image_destroy(&img);

    puts("libtest_image");
    
  }

}
