#include "libmaix_disp.h"

inline static unsigned char make8color(unsigned char r, unsigned char g, unsigned char b)
{
	return (
	(((r >> 5) & 7) << 5) |
	(((g >> 5) & 7) << 2) |
	 ((b >> 6) & 3)	   );
}

inline static unsigned short make16color(unsigned char r, unsigned char g, unsigned char b)
{
	return (
	(((r >> 3) & 31) << 11) |
	(((g >> 2) & 63) << 5)  |
	 ((b >> 3) & 31)		);
}

int main(int argc, char **argv)
{
    struct libmaix_disp *disp = libmaix_disp_create(0);

    if(NULL == disp) {
        printf("creat disp fail\n");
        return -1;
    }

    #define IMG_W 1024
    #define IMG_H 1024
    #define IMG_B 4

    unsigned char test[IMG_W*IMG_H*IMG_B];

    printf("[lcd] w %d h %d bpp %d\r\n", disp->width, disp->height, disp->bpp);
    
    if (disp->bpp == 4) {
      // maybe is ARGB RGBA etc...
      unsigned int *argb = (unsigned int *)test;
      for (int i = 0, sum = disp->width * disp->height; i < sum; i++) {
        argb[i] = 0xFF0000FF; // maybe is R + A or A + B
      }
      disp->draw(disp, test);
    }

    if (disp->bpp == 3) {
      // maybe is RGB BGR etc...
      unsigned char *rgb = (unsigned char *)test;
      for (int i = 0, sum = disp->width * disp->height * disp->bpp; i < sum; i += disp->bpp) {
        rgb[i + 0] = 0xFF; // maybe is RGB or BGR
        rgb[i + 1] = 0xFF;
        rgb[i + 2] = 0x00;
      }
      disp->draw(disp, test);
    }

    if (disp->bpp == 2 || disp->bpp == 1) {
      // maybe is RGB565 etc...
      // use make16color or make8color
    }

    libmaix_disp_destroy(&disp);

    return 0;
}
