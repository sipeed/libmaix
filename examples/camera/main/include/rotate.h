
#ifndef __rotate_H
#define __rotate_H

/******************************************************
 *YUV422：Y：U：V=2:1:1
 *RGB24 ：B G R
******************************************************/
int YUV422PToRGB24(void *RGB24, void *YUV422P, int width, int height)
{
  unsigned char *src_y = (unsigned char *)YUV422P;
  unsigned char *src_u = (unsigned char *)YUV422P + width * height;
  unsigned char *src_v = (unsigned char *)YUV422P + width * height * 3 / 2;

  unsigned char *dst_RGB = (unsigned char *)RGB24;

  int temp[3];

  if (RGB24 == NULL || YUV422P == NULL || width <= 0 || height <= 0)
  {
    printf(" YUV422PToRGB24 incorrect input parameter!\n");
    return -1;
  }

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int Y = y * width + x;
      int U = Y >> 1;
      int V = U;

      temp[0] = src_y[Y] + ((7289 * src_u[U]) >> 12) - 228;                             //b
      temp[1] = src_y[Y] - ((1415 * src_u[U]) >> 12) - ((2936 * src_v[V]) >> 12) + 136; //g
      temp[2] = src_y[Y] + ((5765 * src_v[V]) >> 12) - 180;                             //r

      dst_RGB[3 * Y] = (temp[0] < 0 ? 0 : temp[0] > 255 ? 255
                                                        : temp[0]);
      dst_RGB[3 * Y + 1] = (temp[1] < 0 ? 0 : temp[1] > 255 ? 255
                                                            : temp[1]);
      dst_RGB[3 * Y + 2] = (temp[2] < 0 ? 0 : temp[2] > 255 ? 255
                                                            : temp[2]);
    }
  }

  return 0;
}

int YUV422PToGray(unsigned char *Gray24, unsigned char *YUV422P, int width, int height) __attribute__((optimize("O2")));
int YUV422PToGray(unsigned char *Gray24, unsigned char *YUV422P, int width, int height)
{
  for (int i = 1, sum = width * height * 3 + 1; i != sum; i += 3)
  {
    *Gray24++ = *Gray24++ = *Gray24++ = *YUV422P++; // 0 1 2 = 0
  }
}

unsigned char *cpu_rotate_3(unsigned char *i, int ox, int oy, int rot) __attribute__((optimize("O2")));
unsigned char *cpu_rotate_3(unsigned char *i, int ox, int oy, int rot)
{
#define cpu_rotate_3_max_ox 1280
#define cpu_rotate_3_max_oy 720
// #define cpu_rotate_3_max_ox 256
// #define cpu_rotate_3_max_oy 256

  if (ox > cpu_rotate_3_max_ox || oy > cpu_rotate_3_max_oy)
    return NULL;
  static char n[cpu_rotate_3_max_ox * cpu_rotate_3_max_oy * 3] = {};

  unsigned char *p, *b = i;
  int x, y;

  switch (rot)
  {
  case 1: /* 90 deg right */
    p = n + (oy - 1) * 3;
    for (y = 0; y != oy; y++, p -= 3)
    {
      unsigned char *r = p;
      for (x = 0; x != ox; x++, r += oy * 3)
      {
        r[0] = *(i++);
        r[1] = *(i++);
        r[2] = *(i++);
      }
    }
    break;
  case 2: /* 180 deg */
    i += ox * oy * 3;
    p = n;
    for (y = ox * oy; y > 0; y--)
    {
      i -= 3;
      p[0] = i[0];
      p[1] = i[1];
      p[2] = i[2];
      p += 3;
    }
    break;
  case 3: /* 90 deg left */
    p = n;
    for (y = 0; y != oy; y++, p += 3)
    {
      unsigned char *r = p + ((ox * 3) * oy);
      for (x = 0; x != ox; x++)
      {
        r -= oy * 3;
        r[0] = *(i++);
        r[1] = *(i++);
        r[2] = *(i++);
      }
    }
    break;
  }
  // memcpy(b, n, ox * oy * 3);
  return n;
}

unsigned char *cpu_rotate_1(unsigned char *i, int ox, int oy, int rot) __attribute__((optimize("O2")));
unsigned char *cpu_rotate_1(unsigned char *i, int ox, int oy, int rot)
{
#define cpu_rotate_1_max_ox 640
#define cpu_rotate_1_max_oy 480

  if (ox > cpu_rotate_1_max_ox || oy > cpu_rotate_1_max_oy)
    return NULL;
  static char n[cpu_rotate_1_max_ox * cpu_rotate_1_max_oy] = {};

  unsigned char *p, *b = i;
  int x, y;

  switch (rot)
  {
  case 1: /* 90 deg right */
    p = n + (oy - 1);
    for (y = 0; y != oy; y++, p -= 1)
    {
      unsigned char *r = p;
      for (x = 0; x != ox; x++, r += oy)
      {
        r[0] = *(i++);
      }
    }
    break;
  case 2: /* 180 deg */
    i += ox * oy;
    p = n;
    for (y = ox * oy; y > 0; y--)
    {
      i -= 1;
      p[0] = i[0];
      p += 1;
    }
    break;
  case 3: /* 90 deg left */
    p = n;
    for (y = 0; y != oy; y++, p += 1)
    {
      unsigned char *r = p + ((ox) * oy);
      for (x = 0; x != ox; x++)
      {
        r -= oy;
        r[0] = *(i++);
      }
    }
    break;
  }
  return n;
}

#endif
