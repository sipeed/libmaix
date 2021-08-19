
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "time.h"

#include "libmaix_cam.h"
// #include "libmaix_image.h"
// #include "libmaix_disp.h"
// #include "fb_display.h"

#include "rotate.h"

// #include "plat_math.h"
// #include "log/log.h"

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

static struct timeval {
  long tv_sec;
  long tv_usec;
} old, now;

void cap_set()
{
  gettimeofday(&old, NULL);
}

void cap_get(const char *tips)
{
  return ;
  gettimeofday(&now, NULL);
  if (now.tv_usec > old.tv_usec)
    ALOGE("%20s - %5d us\r\n", tips, (now.tv_usec - old.tv_usec));
}

struct {
  int w0, h0;
  struct libmaix_cam *cam0;
  uint8_t *yuv_buf0;
  void *yuv_ptr0;
  uint8_t *rgb_buf0;
  void *rgb_ptr0;

  int w1, h1;
  struct libmaix_cam *cam1;
  uint8_t *rgb_buf1;
  void *rgb_ptr1;
  uint8_t *yuv_buf1;
  void *yuv_ptr1;

  int w2, h2;
  struct libmaix_cam *cam2;
  uint8_t *yuv_buf2;
  uint8_t *rgb_buf2;

  int w_vo, h_vo;
  struct libmaix_vo *vo;
  uint32_t *argb_vo;
  // struct libmaix_disp *disp;
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

void draw_rectangle(int *buffer, int width, int height, int x, int y, int w, int h, int argb, int thickness)
{
  int rect_w = w, rect_h = h;

  int *addr1 = NULL, *addr2 = NULL;

  if (x < 0)
    x = 0;
  if ((x + rect_w + thickness) >= width)
  {
    rect_w = (width - thickness - x);
  }

  if (y < 0)
    y = 0;
  rect_h += thickness;
  if ((y + rect_h + thickness) >= height)
  {
    rect_h = (height - thickness - y);
  }
  
  for (int i = 0; i < thickness; i++)
  {
    addr1 = (unsigned int *)((unsigned int *)buffer + ((y + i) * width + x));
    addr2 = (unsigned int *)((unsigned int *)buffer + ((y + i + rect_h - thickness) * width + x));

    for (int j = 0; j < rect_w; j++)
    {
      *addr1++ = argb;
      *addr2++ = argb;
    }
  }

  for (int i = 0; i < rect_h; i++)
  {
    addr2 = (unsigned int *)((unsigned int *)buffer + ((y + i) * width + x + rect_w));
    addr1 = (unsigned int *)((unsigned int *)buffer + ((y + i) * width + x));

    for (int j = 0; j < thickness; j++)
    {
      *addr1++ = argb;
      *addr2++ = argb;
    }
  }
}

void test_init() {
  libmaix_camera_module_init();

  test.w0 = 240, test.h0 = 240;

  test.cam0 = libmaix_cam_create(0, test.w0, test.h0, 1, 0);
  if (NULL == test.cam0) return ;
  test.yuv_buf0 = malloc(test.w0 * test.h0 * 3 / 2);
  if (NULL == test.yuv_buf0) return ;
  // test.yuv_ptr0 = nna_creat_yuv(test.yuv_buf0, test.w0, test.h0);
  // if (NULL == test.yuv_ptr0) return ;
  test.rgb_buf0 = malloc(test.w0 * test.h0 * 3);
  if (NULL == test.rgb_buf0) return ;
  // test.rgb_ptr0 = nna_creat_rgb(test.rgb_buf0, test.w0, test.h0);
  // if (NULL == test.rgb_ptr0) return ;
  
  test.w1 = 256, test.h1 = 160;
  
  test.cam1 = libmaix_cam_create(1, test.w1, test.h1, 1, 0);
  if (NULL == test.cam1) return ;
  test.yuv_buf1 = malloc(test.w1 * test.h1 * 3 / 2);
  if (NULL == test.yuv_buf1) return ;
  // test.yuv_ptr1 = nna_creat_yuv(test.yuv_buf1, test.w1, test.h1);
  // if (NULL == test.yuv_ptr1) return ;
  test.rgb_buf1 = malloc(test.w1 * test.h1 * 3);
  if (NULL == test.rgb_buf1) return ;
  // test.rgb_ptr1 = nna_creat_rgb(test.rgb_buf1, test.w1, test.h1);
  // if (NULL == test.rgb_ptr1) return ;
  
  // test.w2 = 640, test.h2 = 480;
  
  // test.cam2 = libmaix_cam_create(2, test.w2, test.h2, 1, 0);
  // if (NULL == test.cam2) return ;
  // test.yuv_buf2 = malloc(test.w2 * test.h2 * 2);
  // if (NULL == test.yuv_buf2) return ;
  // test.rgb_buf2 = malloc(test.w2 * test.h2 * 3);
  // if (NULL == test.rgb_buf2) return ;
  
  test.w_vo = 240, test.h_vo = 240;
  
  test.vo = libmaix_vo_create(test.w0, test.h0, 0, 0, test.w_vo, test.h_vo);
  if (NULL == test.vo) return ;

  test.argb_vo = malloc(test.w_vo * test.h_vo * 4);
  if (NULL == test.argb_vo) return ;

  test.is_run = 1;

  // ALOGE(__FUNCTION__);
}

void test_exit() {

  if (NULL != test.cam0) libmaix_cam_destroy(&test.cam0);
  if (NULL != test.yuv_buf0) free(test.yuv_buf0), test.yuv_buf0 = NULL;
  if (NULL != test.yuv_ptr0) free(test.yuv_ptr0), test.yuv_ptr0 = NULL;
  if (NULL != test.rgb_buf0) free(test.rgb_buf0), test.rgb_buf0 = NULL;
  if (NULL != test.rgb_ptr0) free(test.rgb_ptr0), test.rgb_ptr0 = NULL;

  if (NULL != test.cam1) libmaix_cam_destroy(&test.cam1);
  if (NULL != test.yuv_buf1) free(test.yuv_buf1), test.yuv_buf1 = NULL;
  if (NULL != test.yuv_ptr1) free(test.yuv_ptr1), test.yuv_ptr1 = NULL;
  if (NULL != test.rgb_buf1) free(test.rgb_buf1), test.rgb_buf1 = NULL;
  if (NULL != test.rgb_ptr1) free(test.rgb_ptr1), test.rgb_ptr1 = NULL;

  if (NULL != test.cam2) libmaix_cam_destroy(&test.cam2);
  if (NULL != test.yuv_buf2) free(test.yuv_buf2), test.yuv_buf2 = NULL;
  if (NULL != test.rgb_buf2) free(test.rgb_buf2), test.rgb_buf2 = NULL;

  if (NULL != test.vo) libmaix_vo_destroy(&test.vo), test.vo = NULL;

  if (NULL != test.argb_vo) free(test.argb_vo), test.argb_vo = NULL;

  libmaix_camera_module_deinit();

  printf("--program end");
  // ALOGE(__FUNCTION__);
}

// temp function
void draw_image_3(uint32_t *dst, int ww, int hh, uint8_t *src, int x, int y, int w, int h)
{
  // printf("%d %d %d %d\r\n", ww, hh, w, h);
  int width = ww, height = hh;
  for(int xx = 0; xx < w; ++xx)
  {
    // if (xx > w) break;
    for(int yy = 0; yy < h; ++yy)
    {
      uint8_t *buf = &src[((xx) * (h) + (yy)) * 3];
      dst[(xx) * (width) + (yy)] = ((buf[0] < 120) ? 0x00000000 : 0x7f000000 | (buf[2] << 16) | (buf[1] << 8) | (buf[0]));
    //   dst[(xx) * (width) + (yy)] = 0x7f0000ff;
    //   if (yy > h) break;
    //   if (xx < w || yy < h) {
    //     dst[(xx) * (width) + (yy)] = 0x7f0000ff;// | src[(xx) * (w) + (yy)];
    //   }
    }
  }
}

// temp function
void draw_image_1(uint32_t *dst, int ww, int hh, uint8_t *src, int x, int y, int w, int h)
{
  // printf("%d %d %d %d\r\n", ww, hh, w, h);
  int width = ww, height = hh;
  for(int xx = 0; xx < w; ++xx)
  {
    // if (xx > w) break;
    for(int yy = 0; yy < h; ++yy)
    {
      uint8_t *buf = &src[((xx) * (h) + (yy))];
      dst[(xx) * (width) + (yy)] = ((buf[0] < 120) ? 0x00000000 : 0x7f000000 | (buf[0] << 16) | (buf[0] << 8) | (buf[0]));
    //   dst[(xx) * (width) + (yy)] = 0x7f0000ff;
    //   if (yy > h) break;
    //   if (xx < w || yy < h) {
    //     dst[(xx) * (width) + (yy)] = 0x7f0000ff;// | src[(xx) * (w) + (yy)];
    //   }
    }
  }
}

void test_work() {

  test.cam0->start_capture(test.cam0);
  test.cam1->start_capture(test.cam1);
  // test.cam2->start_capture(test.cam2);

  CALC_FPS("maix_test");
  
  uint8_t *buf = NULL;
  while (test.is_run)
  {
    while (test.is_run)
    {
      if (LIBMAIX_ERR_NONE == test.cam0->capture(test.cam0, test.yuv_buf0))
      {
        // cap_set();
        // VIDEO_FRAME_INFO_S *tmp = vo_get(0);
        // if (tmp) {
        //   memcpy(tmp->VFrame.mpVirAddr[0], test.yuv_buf0, test.w0 * test.h0 * 3 / 2);
        //   vo_set(tmp, 0);
        // }
        // cap_get("display");
        
        cap_set();
        void *frame = test.vo->get_frame(test.vo, 0);
        if (frame != NULL) {
          unsigned int *addr = NULL;
          test.vo->frame_addr(test.vo, frame, &addr, NULL);
          memcpy((void *)addr[0], test.yuv_buf0, test.w0 * test.h0 * 3 / 2);
          test.vo->set_frame(test.vo, frame, 0);
        }
        cap_get("1 display");

        // cap_set();
        // g2d_nv21_rotate(test.yuv_buf0, test.w0, test.h0, 3);
        // cap_get("g2d_nv21_rotate");

        // nna_covert_yuv(test.yuv_ptr0, test.rgb_ptr0);
        break;

        // cap_set();
        // buf = cpu_rotate_3(test.rgb_buf0, test.w0, test.h0, 3);
        // memcpy(test.rgb_buf0, buf, test.w0 * test.h0 * 3);
        // cap_get("0 cpu_rotate");

        // cap_set();
        // fb_display(test.rgb_buf0, 0, test.h0, test.w0, 0, 0, (600 - test.h0) / 2, (1024 - test.w0) / 2);
        // cap_get("fb_display");
          
      }
      usleep(50 * 1000);
    }

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
