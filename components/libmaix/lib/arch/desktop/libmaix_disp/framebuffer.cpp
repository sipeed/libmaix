#include "libmaix_disp_priv.h"
#include "libmaix_err.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/ioctl.h>

#ifndef FBDEV_PATH
#define FBDEV_PATH "/dev/fb0"
#endif

static libmaix_err_t disp_draw_image(struct libmaix_disp *disp, struct libmaix_image *img)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;
  // printf("disp_draw_image %d %d %p \r\n", img->width, img->height, priv->disp_img);

  // if (priv->disp_img == NULL) {
  //   // bind disp->bpp && LIBMAIX_IMAGE_MODE_RGB565
  //   libmaix_image_mode_t mode = LIBMAIX_IMAGE_MODE_INVALID;
  //   switch (disp->bpp) {
  //     case 4: mode = LIBMAIX_IMAGE_MODE_RGBA8888; break;
  //     // case 3: mode = LIBMAIX_IMAGE_MODE_RGB888; break;
  //     case 3: mode = LIBMAIX_IMAGE_MODE_BGR888; break; // v83x fb set bgr888 is bug!!!! : (
  //     case 2: mode = LIBMAIX_IMAGE_MODE_RGB565; break;
  //   }
  //   priv->disp_img = libmaix_image_create(disp->width, disp->height, mode, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
  //   if(!priv->disp_img) return LIBMAIX_ERR_NO_MEM;
  // }

  if (img->mode == LIBMAIX_IMAGE_MODE_RGB888){
      cv::Mat frame(img->height, img->width, CV_8UC3, img->data);
      // cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
      cv::imshow("[framebuffer]", frame);
  }

  // if (img->mode != priv->disp_img->mode){
  //     if (LIBMAIX_ERR_NONE != img->convert(img, priv->disp_img->mode, &priv->disp_img))
  //     {
  //         return LIBMAIX_ERR_NOT_IMPLEMENT;
  //     }
  //     memcpy((unsigned char *)priv->fbp, priv->disp_img->data, disp->width * disp->height * disp->bpp);
  // } else {
  //     memcpy((unsigned char *)priv->fbp, img->data, disp->width * disp->height * disp->bpp);
  // }

  // priv->vinfo.yoffset = 0;
  // priv->vinfo.reserved[0] = 0;
  // priv->vinfo.reserved[1] = 0;
  // priv->vinfo.reserved[2] = disp->width;
  // priv->vinfo.reserved[3] = disp->height;

  // if (priv->fbiopan) {
  //   if (ioctl(priv->fbfd, FBIOPAN_DISPLAY, &priv->vinfo))
  //   {
  //     fprintf(stderr, "ioctl FBIOPAN_DISPLAY: %s\n", strerror(errno));
  //     return LIBMAIX_ERR_UNKNOWN;
  //   }
  // }

  return LIBMAIX_ERR_NONE;
}

static libmaix_err_t disp_draw(struct libmaix_disp *disp, unsigned char *buf)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  // memcpy((unsigned char *)priv->fbp, buf, disp->width * disp->height * disp->bpp);

  // priv->vinfo.yoffset = 0;
  // priv->vinfo.reserved[0] = 0;
  // priv->vinfo.reserved[1] = 0;
  // priv->vinfo.reserved[2] = disp->width;
  // priv->vinfo.reserved[3] = disp->height;

  // if (priv->fbiopan) {
  //   if (ioctl(priv->fbfd, FBIOPAN_DISPLAY, &priv->vinfo))
  //   {
  //     fprintf(stderr, "ioctl FBIOPAN_DISPLAY: %s\n", strerror(errno));
  //     return LIBMAIX_ERR_UNKNOWN;
  //   }
  // }

  return LIBMAIX_ERR_NONE;
}

static int priv_devDeinit(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;
  // if (priv->fbp)
  {
    // munmap(priv->fbp, priv->finfo.smem_len);
    // close(priv->fbfd);
    if(priv->disp_img != NULL) {
        libmaix_image_destroy(&priv->disp_img);
    }
  }
  return 0;
}

static int priv_devInit(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  // if (priv->fbp == NULL)
  {
    // priv->fbfd = open(FBDEV_PATH, O_RDWR);
    // if (-1 == priv->fbfd)
    // {
    //   fprintf(stderr, "open %s fail\n", FBDEV_PATH);
    //   return -1;
    // }

    // // Get fixed screen information
    // if (ioctl(priv->fbfd, FBIOGET_FSCREENINFO, &priv->finfo) == -1)
    // {
    //   fprintf(stderr, "Error reading fixed information");
    //   return -1;
    // }

    // // Get variable screen information
    // if (ioctl(priv->fbfd, FBIOGET_VSCREENINFO, &priv->vinfo) == -1)
    // {
    //   fprintf(stderr, "Error reading variable information");
    //   return -1;
    // }

    // printf("[framebuffer](%d,%d, %dbpp)\n", priv->vinfo.xres, priv->vinfo.yres, priv->vinfo.bits_per_pixel);

    // // Map the device to memory
    // priv->fbp = (char *)mmap(0, priv->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, priv->fbfd, 0);
    // if ((intptr_t)priv->fbp == -1)
    // {
    //   fprintf(stderr, "Error: failed to map framebuffer device to memory");
    //   return -1;
    // }

    // disp->width = priv->vinfo.xres;
    // disp->height = priv->vinfo.yres;
    // disp->bpp = priv->vinfo.bits_per_pixel / 8;

    disp->width = INT32_MAX;
    disp->height = INT32_MAX;
    disp->bpp = 3; // RGB888

    // printf("The framebuffer device was mapped to memory successfully.\n");
    cv::namedWindow( "[framebuffer]", cv::WINDOW_AUTOSIZE );
    cv::startWindowThread(); // if( (char)cv::waitKey(33) >= 0 ) LIBMAIX_ERR_NONE;

    return LIBMAIX_ERR_NONE;
  }

  return LIBMAIX_ERR_UNKNOWN;
}

int disp_priv_init(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  disp->draw = disp_draw;
  disp->draw_image = disp_draw_image;

  priv->disp_img = NULL;
  priv->devInit = priv_devInit;
  priv->devDeinit = priv_devDeinit;

  return priv->devInit(disp);
}

#ifdef __cplusplus
}
#endif
