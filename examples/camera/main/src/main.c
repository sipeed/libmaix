
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

#include "sys/time.h"

static struct timeval old, now;

static void cap_set()
{
  gettimeofday(&old, NULL);
}

static void cap_get(const char *tips)
{
  gettimeofday(&now, NULL);
  if (now.tv_usec > old.tv_usec)
    printf("%20s - %5ld ms\r\n", tips, (now.tv_usec - old.tv_usec) / 1000);
}

struct {
  int w0, h0, w1, h1;
  struct libmaix_cam *cam0;
  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  struct libmaix_cam *cam1;
  #endif
  uint8_t *rgb888;

  struct libmaix_disp *disp;

  int is_run;
} app = { 0 };

static void app_handlesig(int signo)
{
  if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo || SIGQUIT == signo || SIGPIPE == signo || SIGKILL == signo)
  {
    app.is_run = 0;
  }
  // exit(0);
}

void app_init() {

  libmaix_camera_module_init();

  app.w0 = 224, app.h0 = 224;

  app.cam0 = libmaix_cam_create(0, app.w0, app.h0, 1, 0);
  if (NULL == app.cam0) return ;

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  app.w1 = 320, app.h1 = 240;
  app.cam1 = libmaix_cam_create(1, app.w1, app.h1, 1, 0);
  if (NULL == app.cam1) return ;
  app.rgb888 = (uint8_t *)malloc(app.w0 * app.h0 * 3);
  if (app.rgb888 == NULL) return ;
  #endif

  app.disp = libmaix_disp_create(0);
  if(NULL == app.disp) return ;
  if (app.disp->width == 0 || app.disp->height == 0) app.disp->width = app.w0, app.disp->height = app.h0;

  app.is_run = 1;

  // ALOGE(__FUNCTION__);
}

void app_exit() {

  if (NULL != app.cam0) libmaix_cam_destroy(&app.cam0);

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  if (NULL != app.cam1) libmaix_cam_destroy(&app.cam1);
  #endif

  if (NULL != app.rgb888) free(app.rgb888), app.rgb888 = NULL;
  if (NULL != app.disp) libmaix_disp_destroy(&app.disp), app.disp = NULL;

  libmaix_camera_module_deinit();

  // ALOGE(__FUNCTION__);
}

void app_work() {

  app.cam0->start_capture(app.cam0);

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  app.cam1->start_capture(app.cam1);
  #endif

  while (app.is_run)
  {
    // goal code
    libmaix_image_t *tmp = NULL;
    if (LIBMAIX_ERR_NONE == app.cam0->capture_image(app.cam0, &tmp))
    {
        printf("w %d h %d p %d \r\n", tmp->width, tmp->height, tmp->mode);

        if (tmp->width == app.disp->width && app.disp->height == tmp->height) {
            app.disp->draw_image(app.disp, tmp);
        } else {
            libmaix_image_t *rs = libmaix_image_create(app.disp->width, app.disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
            if (rs) {
                libmaix_cv_image_resize(tmp, app.disp->width, app.disp->height, &rs);
                app.disp->draw_image(app.disp, rs);
                libmaix_image_destroy(&rs);
            }
        }
        CALC_FPS("maix_cam 0");

        #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
        libmaix_image_t *t = NULL;
        if (LIBMAIX_ERR_NONE == app.cam1->capture_image(app.cam1, &t))
        {
            printf("w %d h %d p %d \r\n", t->width, t->height, t->mode);
            CALC_FPS("maix_cam 1");
        }
        #endif
    }
  }

}

int main(int argc, char **argv)
{
  signal(SIGINT, app_handlesig);
  signal(SIGTERM, app_handlesig);

  libmaix_image_module_init();

  app_init();
  app_work();
  app_exit();


  libmaix_image_module_deinit();

  return 0;
}
