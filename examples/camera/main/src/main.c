
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "time.h"

#include "libmaix_cam.h"
// #include "libmaix_image.h"
#include "libmaix_disp.h"

// #include "rotate.h"

#define CALC_FPS(tips)                                                                                     \
  {                                                                                                        \
    static int fcnt = 0;                                                                                   \
    fcnt++;                                                                                                \
    static struct timespec ts1, ts2;                                                                       \
    clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
    if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
    {                                                                                                      \
      printf("%s => H26X FPS:%d\n", tips, fcnt);                                                  \
      ts1 = ts2;                                                                                           \
      fcnt = 0;                                                                                            \
    }                                                                                                      \
  }

static struct timeval old, now;

void cap_set()
{
  gettimeofday(&old, NULL);
}

void cap_get(const char *tips)
{
  return ;
  gettimeofday(&now, NULL);
  if (now.tv_usec > old.tv_usec)
    printf("%20s - %5ld us\r\n", tips, (now.tv_usec - old.tv_usec));
}

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

struct {
  int w0, h0;
  struct libmaix_cam *cam0;
  uint8_t *rgb_buf0;
  
  struct libmaix_disp *disp;

  int is_run;
} test = { 0 };

static void test_handlesig(int signo)
{
  if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
  {
    test.is_run = 0;
  }
  // exit(0);
}

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

void test_init() {

  libmaix_camera_module_init();

  test.w0 = 240, test.h0 = 240;

  test.cam0 = libmaix_cam_create(0, test.w0, test.h0, 0, 0);
  if (NULL == test.cam0) return ;
  test.rgb_buf0 = malloc(test.w0 * test.h0 * 3);
  
  test.disp = libmaix_disp_create();
  if(NULL == test.disp) return ;

  test.is_run = 1;

  // ALOGE(__FUNCTION__);
}

void test_exit() {

  if (NULL != test.cam0) libmaix_cam_destroy(&test.cam0);
  if (NULL != test.rgb_buf0) free(test.rgb_buf0), test.rgb_buf0 = NULL;
  if (NULL != test.disp) libmaix_disp_destroy(&test.disp), test.disp = NULL;

  libmaix_camera_module_deinit();

  // ALOGE(__FUNCTION__);
}

void test_work() {

  test.cam0->start_capture(test.cam0);

  // unsigned short rgb565[test.disp->width * test.disp->height];

  while (test.is_run)
  {
    if (LIBMAIX_ERR_NONE == test.cam0->capture(test.cam0, test.rgb_buf0))
    {
      // if (test.disp->bpp == 2 || test.disp->bpp == 1) {
      //   uint8_t tmp[test.disp->width * test.disp->height * 2];
      //   unsigned short *rgb565 = (unsigned short *)tmp;
      //   for (int i = 0, sum = test.disp->width * test.disp->height; i < sum; i++) {
      //     rgb565[i] = make16color(test.rgb_buf0[i + 0], test.rgb_buf0[i + 1], test.rgb_buf0[i + 2]);
      //   }
      //   test.disp->draw(test.disp, rgb565);
      // }
    }
    // usleep(20 * 1000);
    CALC_FPS("maix_test");
  }

}

int main(int argc, char **argv)
{
  signal(SIGINT, test_handlesig);
  signal(SIGTERM, test_handlesig);

  test_init();
  test_work();
  test_exit();

  return 0;
}
