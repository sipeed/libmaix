
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
  int w0, h0;
  struct libmaix_cam *cam0;
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

  app.w0 = 854, app.h0 = 480;

  app.cam0 = libmaix_cam_create(0, app.w0, app.h0, 1, 0);
  if (NULL == app.cam0) return ;

  app.disp = libmaix_disp_create(0);

  if(NULL == app.disp) return ;
  if (app.disp->width == 0 || app.disp->height == 0) app.disp->width = app.w0, app.disp->height = app.h0;

  app.is_run = 1;

}

void app_exit() {

  if (NULL != app.cam0) libmaix_cam_destroy(&app.cam0);
  if (NULL != app.disp) libmaix_disp_destroy(&app.disp), app.disp = NULL;

  libmaix_camera_module_deinit();
}

void app_work() {

  app.cam0->start_capture(app.cam0);

  while (app.is_run)
  {
    libmaix_image_t *tmp = NULL;
    if (LIBMAIX_ERR_NONE == app.cam0->capture_image(app.cam0, &tmp))
    {
        printf("w %d %d h %d %d p %d d %p \r\n", app.disp->width, tmp->width, app.disp->height, tmp->height, tmp->mode, tmp->data);

        app.disp->draw_image(app.disp, tmp);

        CALC_FPS("maix_cam 0");
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
