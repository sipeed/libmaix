#include "libjpeg.h"

#include "cdjpeg.h"
#include "jversion.h"

/* 0: ok, 1: failed */
uint8_t libjpeg_compress(jpeg_img_t *img, uint8_t quality, uint8_t **jpeg_buf, uint64_t *jpeg_len)
{
  uint32_t img_width = img->w;
  uint32_t img_height = img->h;
  uint32_t img_pixels = img->bpp;
  uint8_t *img_data = (uint8_t*)(img->data);

  uint32_t old = (uint32_t)(*jpeg_buf);

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  cinfo.image_width = img_width;
  cinfo.image_height = img_height;
  cinfo.input_components = img_pixels;
  cinfo.in_color_space = JCS_RGB;
  jpeg_mem_dest(&cinfo, jpeg_buf, (unsigned long *)jpeg_len);
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  while (cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer[0] = &img_data[cinfo.next_scanline * img_width * 3];
    (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);
  if((uint32_t)*jpeg_buf != old) {
    printf("memory oveflow\r\n");
    free(*jpeg_buf);
    return 1;
  }
  return (cinfo.err->msg_code != 0);
}

/* 0: ok, 1: failed */
uint8_t libjpeg_decompress(jpeg_img_t *jpeg, uint8_t *jpeg_buf, uint32_t jpeg_len)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buffer;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  jpeg_mem_src(&cinfo, jpeg_buf, jpeg_len);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  memset(jpeg, 0, sizeof(jpeg_img_t));

  jpeg->w = cinfo.output_width;
  jpeg->h = cinfo.output_height;
  jpeg->bpp = cinfo.output_components;
  
  if(jpeg->data) {
    free(jpeg->data);
    jpeg->data = NULL;
  }

  jpeg->data = malloc(jpeg->w * jpeg->h * jpeg->bpp);
  if(NULL == jpeg->data) {
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 1;
  }
  // memset(jpeg->data, 0, jpeg->w * jpeg->h * jpeg->bpp);

  uint32_t row_size = jpeg->w * jpeg->bpp;
  uint8_t *point = jpeg->data;

  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_size, 1);

  while (cinfo.output_scanline < jpeg->h)
  {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy(point, *buffer, row_size);
    point += row_size;
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return (cinfo.err->msg_code != JTRC_EOI);
}

//By Tongxiaohua@20210806
void libjpeg_decompress_free(jpeg_img_t *jpeg)
{
	if(NULL == jpeg)
	{
		return ;
	}

	if(NULL != jpeg->data){

		free(jpeg->data);
		jpeg->data = NULL;
	}


	return ;
	
}

