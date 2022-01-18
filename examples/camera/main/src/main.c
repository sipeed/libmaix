
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "time.h"

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "libmaix_cam.h"
#include "libmaix_image.h"
#include "libmaix_disp.h"

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

#include <sys/time.h>

static struct timeval old, now;

void cap_set()
{
  gettimeofday(&old, NULL);
}

void cap_get(const char *tips)
{
  gettimeofday(&now, NULL);
  if (now.tv_usec > old.tv_usec)
    printf("%20s - %ld us\r\n", tips, (now.tv_usec - old.tv_usec));
}

void draw_rectangle(uint32_t *buffer, int width, int height, int x, int y, int w, int h, int argb, int thickness)
{
  int rect_w = w, rect_h = h;

  uint32_t *addr1 = NULL, *addr2 = NULL;

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

struct {
  int w0, h0;
  struct libmaix_cam *cam0;
  struct libmaix_cam *cam1;
  uint8_t *yuv420;

  // struct libmaix_disp *disp;

  int w_vo, h_vo;
  struct libmaix_vo *vo;
  uint32_t *argb_vo;

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

void test_init() {

  libmaix_camera_module_init();

  test.w0 = 320, test.h0 = 240;

  test.cam0 = libmaix_cam_create(0, test.w0, test.h0, 0, 0);
  if (NULL == test.cam0) return ;

  test.cam1 = libmaix_cam_create(1, test.w0, test.h0, 0, 0);
  if (NULL == test.cam1) return ;

  test.yuv420 = (uint8_t *)malloc(test.w0 * test.h0 * 3 / 2);
  if (NULL == test.yuv420) return ;

  test.w_vo = 240, test.h_vo = 180;

  test.vo = libmaix_vo_create(test.w0, test.h0, 0, 0, test.w_vo, test.h_vo);
  if (NULL == test.vo) return ;

  test.argb_vo = malloc(test.w_vo * test.h_vo * 4);
  if (NULL == test.argb_vo) return ;

  test.is_run = 1;

  // ALOGE(__FUNCTION__);
}

void test_exit() {

  if (NULL != test.cam0) libmaix_cam_destroy(&test.cam0);

  if (NULL != test.cam1) libmaix_cam_destroy(&test.cam1);

  if (NULL != test.yuv420) free(test.yuv420), test.yuv420 = NULL;

  if (NULL != test.vo)
    libmaix_vo_destroy(&test.vo), test.vo = NULL;

  if (NULL != test.argb_vo)
    free(test.argb_vo), test.argb_vo = NULL;

  libmaix_camera_module_deinit();

  // ALOGE(__FUNCTION__);
}

void test_work() {

  test.cam0->start_capture(test.cam0);
  test.cam1->start_capture(test.cam1);

  void * p_vir_addr = (void *)g2d_allocMem(test.w0 * test.h0 * 4);
  void * p_phy_addr = (g2d_getPhyAddrByVirAddr(p_vir_addr));

  void * n_vir_addr = (void *)g2d_allocMem(test.w0 * test.h0);
  void * v_vir_addr = (void *)g2d_allocMem(test.w0 * test.h0 / 2);
  void * n_phy_addr = (g2d_getPhyAddrByVirAddr(n_vir_addr));
  void * v_phy_addr = (g2d_getPhyAddrByVirAddr(v_vir_addr));

  int ret = 0, _w_h_ = test.w0 * test.h0; //ALIGN(w, 16) * ALIGN(h, 16);

  libmaix_cv_apriltag_load();

  while (test.is_run)
  {
    // goal code
    // libmaix_image_t *tmp = NULL;
    if (LIBMAIX_ERR_NONE == test.cam1->capture(test.cam1, test.yuv420))
    {
      CALC_FPS("test_idle");
    }

    if (LIBMAIX_ERR_NONE == test.cam0->capture(test.cam0, test.yuv420))
    {
      CALC_FPS("test_vivo");
      {
        void *frame = test.vo->get_frame(test.vo, 0);
        if (frame != NULL) {
          uint32_t *vir = NULL, *phy = NULL;
          test.vo->frame_addr(test.vo, frame, &vir, &phy);

          // memcpy((void *)test.yuv420, test.yuv420, _w_h_), memcpy((void *)v_vir_addr, test.yuv420 + _w_h_, _w_h_ / 2);

          libmaix_cv_apriltag_test(test.yuv420, test.w0, test.h0);

          // _g2d_nv21_rotate(n_phy_addr, v_phy_addr, (phy[0]), (phy[1]), test.w0, test.h0, 1);

          // g2d_nv21_rotate(test.yuv420, test.w0, test.h0, 1);
          memcpy((void *)vir[0], test.yuv420, test.w0 * test.h0 * 3 / 2);

          test.vo->set_frame(test.vo, frame, 0);
        }
      }
      // {
      //   void *frame = test.vo->get_frame(test.vo, 9);
      //   if (frame) {
      //     uint32_t *vir = NULL, *phy = NULL;
      //     test.vo->frame_addr(test.vo, frame, &vir, &phy);

      //     // draw_rectangle(p_vir_addr, test.h_vo, test.w_vo, 10, 20, 30, 40, 0xef00ff00, 5);

      //     // draw_rectangle(p_vir_addr, test.h_vo, test.w_vo, 20, 10, 40, 30, 0xef0000ff, 5);

      //     // static struct timeval tmp;
      //     // gettimeofday(&tmp, NULL);

      //     // static char t[128] = {};
      //     // sprintf(t, "%ld.%ld\r\n", tmp.tv_sec, tmp.tv_usec);
      //     // // sprintf(t, "%ld\r\n", i++);
      //     // draw_string(p_vir_addr, test.h_vo, test.w_vo, (uint8_t *)t, 20, 20, 32, 0xefff0000, 0x00ffffff);

      //     memset(p_vir_addr, 0, test.h_vo * test.w_vo * 4);

      //     libmaix_cv_image_test(p_vir_addr, n_vir_addr, test.h_vo, test.w_vo);

      //     // cap_set();
      //     _g2d_argb_rotate((void *)p_phy_addr, (void *)phy[0], test.h_vo, test.w_vo, 3);
      //     // cap_get("g2d");

      //     // memcpy(vir[0], test.argb_vo, test.h_vo * test.w_vo);
      //     // printf("p_phy_addr %p\r\n", p_phy_addr);

      //     test.vo->set_frame(test.vo, frame, 9);

      //   }
      // }
    }
  }

  libmaix_cv_apriltag_free();

  g2d_freeMem(p_vir_addr);
  g2d_freeMem(n_vir_addr);
  g2d_freeMem(v_vir_addr);
}

int main(int argc, char **argv)
{
  signal(SIGINT, test_handlesig);
  signal(SIGTERM, test_handlesig);

  libmaix_image_module_init();

  test_init();
  test_work();
  test_exit();

  libmaix_image_module_deinit();

  return 0;
}
