#include "libmaix_disp_priv.h"
#include "libmaix_err.h"

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/ioctl.h>

#ifndef FBDEV_PATH
#define FBDEV_PATH "/dev/fb0"
#endif

static libmaix_err_t disp_draw(struct libmaix_disp *disp, unsigned char *buf)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  memcpy((unsigned char *)priv->fbp, buf, disp->width * disp->height * disp->bpp);

  priv->vinfo.yoffset = 0;
  priv->vinfo.reserved[0] = 0;
  priv->vinfo.reserved[1] = 0;
  priv->vinfo.reserved[2] = disp->width;
  priv->vinfo.reserved[3] = disp->height;

  if (priv->fbiopan) {
    if (ioctl(priv->fbfd, FBIOPAN_DISPLAY, &priv->vinfo))
    {
      fprintf(stderr, "ioctl FBIOPAN_DISPLAY: %s\n", strerror(errno));
      return LIBMAIX_ERR_UNKNOWN;
    }
  }

  return LIBMAIX_ERR_NONE;
}

static int priv_devDeinit(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;
  if (priv->fbp)
  {
    munmap(priv->fbp, priv->finfo.smem_len);
    close(priv->fbfd);
  }
  return 0;
}

static int priv_devInit(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  if (priv->fbp == NULL)
  {
    priv->fbfd = open(FBDEV_PATH, O_RDWR);
    if (-1 == priv->fbfd)
    {
      fprintf(stderr, "open %s fail\n", FBDEV_PATH);
      return -1;
    }

    // Get fixed screen information
    if (ioctl(priv->fbfd, FBIOGET_FSCREENINFO, &priv->finfo) == -1)
    {
      fprintf(stderr, "Error reading fixed information");
      return -1;
    }

    // Get variable screen information
    if (ioctl(priv->fbfd, FBIOGET_VSCREENINFO, &priv->vinfo) == -1)
    {
      fprintf(stderr, "Error reading variable information");
      return -1;
    }

    printf("%dx%d, %dbpp\n", priv->vinfo.xres, priv->vinfo.yres, priv->vinfo.bits_per_pixel);

    // Map the device to memory
    priv->fbp = (char *)mmap(0, priv->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, priv->fbfd, 0);
    if ((intptr_t)priv->fbp == -1)
    {
      fprintf(stderr, "Error: failed to map framebuffer device to memory");
      return -1;
    }

    disp->width = priv->vinfo.xres;
    disp->height = priv->vinfo.yres;
    disp->bpp = priv->vinfo.bits_per_pixel / 8;

    // printf("The framebuffer device was mapped to memory successfully.\n");

    return LIBMAIX_ERR_NONE;
  }

  return LIBMAIX_ERR_UNKNOWN;
}

int disp_priv_init(struct libmaix_disp *disp)
{
  struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)disp->reserved;

  disp->draw = disp_draw;

  priv->devInit = priv_devInit;
  priv->devDeinit = priv_devDeinit;

  return priv->devInit(disp);
}
