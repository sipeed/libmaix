
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
  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  struct libmaix_cam *cam1;
  #endif
  uint8_t *rgb888;

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

void test_init() {

  libmaix_camera_module_init();

  test.w0 = 240, test.h0 = 240;

  test.cam0 = libmaix_cam_create(0, test.w0, test.h0, 1, 0);
  if (NULL == test.cam0) return ;

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  test.cam1 = libmaix_cam_create(1, test.w0, test.h0, 1, 0);
  if (NULL == test.cam1) return ;
  test.rgb888 = (uint8_t *)malloc(test.w0 * test.h0 * 3);
  if (test.rgb888 == NULL) return ;
  #endif

  test.disp = libmaix_disp_create(0);
  if(NULL == test.disp) return ;

  test.is_run = 1;

  // ALOGE(__FUNCTION__);
}

void test_exit() {

  if (NULL != test.cam0) libmaix_cam_destroy(&test.cam0);

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  if (NULL != test.cam1) libmaix_cam_destroy(&test.cam1);
  #endif

  if (NULL != test.rgb888) free(test.rgb888), test.rgb888 = NULL;
  if (NULL != test.disp) libmaix_disp_destroy(&test.disp), test.disp = NULL;

  libmaix_camera_module_deinit();

  // ALOGE(__FUNCTION__);
}

static libmaix_image_t *gray = NULL;

#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "tagCircle21h7.h"
#include "tagCircle49h12.h"
#include "tagCustom48h12.h"
#include "tagStandard41h12.h"
#include "tagStandard52h13.h"

static const char *famname = "tag36h11";
static apriltag_detector_t *td = NULL;
static apriltag_family_t *tf = NULL;

static void apriltag_init()
{
    if (gray == NULL) {
        gray = libmaix_image_create(test.w0, test.h0, LIBMAIX_IMAGE_MODE_GRAY, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        if (gray) {
            // Initialize tag detector with options
            if (!strcmp(famname, "tag36h11")) {
                tf = tag36h11_create();
            } else if (!strcmp(famname, "tag25h9")) {
                tf = tag25h9_create();
            } else if (!strcmp(famname, "tag16h5")) {
                tf = tag16h5_create();
            } else if (!strcmp(famname, "tagCircle21h7")) {
                tf = tagCircle21h7_create();
            } else if (!strcmp(famname, "tagCircle49h12")) {
                tf = tagCircle49h12_create();
            } else if (!strcmp(famname, "tagStandard41h12")) {
                tf = tagStandard41h12_create();
            } else if (!strcmp(famname, "tagStandard52h13")) {
                tf = tagStandard52h13_create();
            } else if (!strcmp(famname, "tagCustom48h12")) {
                tf = tagCustom48h12_create();
            } else {
                printf("Unrecognized tag family name. Use e.g. \"tag36h11\".\n");
                exit(-1);
            }

            td = apriltag_detector_create();
            apriltag_detector_add_family(td, tf);
            td->quad_decimate = 2.0; // "Decimate input image by this factor"
            td->quad_sigma = 0.0; // Apply low-pass blur to input
            td->nthreads = 1;
            td->debug = 0;
            td->refine_edges = 1;// Spend more time trying to align edges of tags

        }
    }
}

static void apriltag_exit()
{

    if (gray) {
        libmaix_image_destroy(&gray);

        apriltag_detector_destroy(td), td = NULL;

        if (!strcmp(famname, "tag36h11")) {
            tag36h11_destroy(tf);
        } else if (!strcmp(famname, "tag25h9")) {
            tag25h9_destroy(tf);
        } else if (!strcmp(famname, "tag16h5")) {
            tag16h5_destroy(tf);
        } else if (!strcmp(famname, "tagCircle21h7")) {
            tagCircle21h7_destroy(tf);
        } else if (!strcmp(famname, "tagCircle49h12")) {
            tagCircle49h12_destroy(tf);
        } else if (!strcmp(famname, "tagStandard41h12")) {
            tagStandard41h12_destroy(tf);
        } else if (!strcmp(famname, "tagStandard52h13")) {
            tagStandard52h13_destroy(tf);
        } else if (!strcmp(famname, "tagCustom48h12")) {
            tagCustom48h12_destroy(tf);
        }
        tf = NULL;
    }

}

static void apriltag_loop(libmaix_image_t* img)
{
    if (gray) {

        libmaix_err_t err = LIBMAIX_ERR_NONE;

        libmaix_cv_image_convert(img, LIBMAIX_IMAGE_MODE_GRAY, &gray);

        // Make an image_u8_t header for the Mat data
        image_u8_t im = {
            .width = gray->width,
            .height = gray->height,
            .stride = gray->width,
            .buf = gray->data
        };

        zarray_t *detections = apriltag_detector_detect(td, &im);

        // Draw detection outlines
        for (int i = 0; i < zarray_size(detections); i++) {
          apriltag_detection_t *det;
          zarray_get(detections, i, &det);

          libmaix_cv_image_draw_line(img, (int)det->p[0][0], (int)det->p[0][1], (int)det->p[1][0], (int)det->p[1][1], MaixColor(0, 0xff, 0), 2);
          libmaix_cv_image_draw_line(img, (int)det->p[0][0], (int)det->p[0][1], (int)det->p[3][0], (int)det->p[3][1], MaixColor(0, 0xff, 0), 2);
          libmaix_cv_image_draw_line(img, (int)det->p[1][0], (int)det->p[1][1], (int)det->p[2][0], (int)det->p[2][1], MaixColor(0, 0xff, 0), 2);
          libmaix_cv_image_draw_line(img, (int)det->p[2][0], (int)det->p[2][1], (int)det->p[3][0], (int)det->p[3][1], MaixColor(0, 0xff, 0), 2);

          char det_id[8] = {0};
          sprintf(det_id, "%d", det->id);
          int w, h;
          libmaix_cv_image_get_string_size(&w, &h, det_id, 1.5, 1);
          libmaix_cv_image_draw_string(img, (int)(det->c[0]), (int)(det->c[1]) - (h / 2), det_id, 1.5, MaixColor(255, 0, 0), 1);

          // printf("\r\n[(%s,%.02f):(%d,%d)]:[(%d:%d),(%d:%d)]:[(%d:%d),(%d:%d)]:[(%d:%d),(%d:%d)]:[(%d:%d),(%d:%d)]\r\n",
          //     det_id, det->decision_margin, (int)det->c[0], (int)det->c[1],
          //     (int)det->p[0][0], (int)det->p[0][1], (int)det->p[1][0], (int)det->p[1][1],
          //     (int)det->p[1][0], (int)det->p[1][1], (int)det->p[3][0], (int)det->p[3][1],
          //     (int)det->p[1][0], (int)det->p[1][1], (int)det->p[2][0], (int)det->p[2][1],
          //     (int)det->p[2][0], (int)det->p[2][1], (int)det->p[3][0], (int)det->p[3][1]
          // );

        }

        apriltag_detections_destroy(detections);

    }
}

#include "zbar.h"
#include "symbol.h"

static zbar_image_scanner_t *scanner = NULL;

static void qrcode_init()
{
    if (gray == NULL) {
        gray = libmaix_image_create(test.w0, test.h0, LIBMAIX_IMAGE_MODE_GRAY, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        if (gray) {
            /* create a reader */
            scanner = zbar_image_scanner_create();
            /* configure the reader */
            // zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
            // zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_POSITION, 1);
            // zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_UNCERTAINTY, 2);
            // zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_CODE128, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_CODE93, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_CODE39, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_CODABAR, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_COMPOSITE, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_PDF417, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_CODABAR, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_COMPOSITE, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_PDF417, ZBAR_CFG_UNCERTAINTY, 0);
            // zbar_image_scanner_set_config(scanner, ZBAR_DATABAR, ZBAR_CFG_UNCERTAINTY, 0);
        }
    }

}

static void qrcode_exit()
{
    if (gray) {
        libmaix_image_destroy(&gray);
        if (scanner != NULL) {
          zbar_image_scanner_destroy(scanner), scanner = NULL;
        }
    }
}

#define zbar_fourcc(a, b, c, d)                 \
        ((unsigned long)(a) |                   \
         ((unsigned long)(b) << 8) |            \
         ((unsigned long)(c) << 16) |           \
         ((unsigned long)(d) << 24))

static void qrcode_loop(libmaix_image_t* img)
{
    if (gray) {
        libmaix_err_t err = LIBMAIX_ERR_NONE;

        libmaix_cv_image_convert(img, LIBMAIX_IMAGE_MODE_GRAY, &gray);

        /* obtain image data */
        int width = gray->width, height = gray->height;
        uint8_t *raw = gray->data;

        /* wrap image data */
        zbar_image_t *image = zbar_image_create();
        zbar_image_set_format(image, zbar_fourcc('Y','8','0','0'));
        zbar_image_set_size(image, width, height);
        zbar_image_set_data(image, raw, width * height, NULL);

        // cap_set();

        /* scan the image for barcodes */
        int n = zbar_scan_image(scanner, image);

        cap_get("zbar_scan_image");

        /* extract results */
        const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
        for(; symbol; symbol = zbar_symbol_next(symbol)) {

            int point_count = zbar_symbol_set_get_size(symbol);
            printf("point:%d",point_count);
            point_t point_rect[4];
            point_rect[0].x = zbar_symbol_get_loc_x(symbol, 0);
            point_rect[0].y = zbar_symbol_get_loc_y(symbol, 0);
            point_rect[1].x = zbar_symbol_get_loc_x(symbol, 1);
            point_rect[1].y = zbar_symbol_get_loc_y(symbol, 1);
            point_rect[2].x = zbar_symbol_get_loc_x(symbol, 2);
            point_rect[2].y = zbar_symbol_get_loc_y(symbol, 2);
            point_rect[3].x = zbar_symbol_get_loc_x(symbol, 3);
            point_rect[3].y = zbar_symbol_get_loc_y(symbol, 3);
            libmaix_cv_image_draw_line(img, point_rect[0].x, point_rect[0].y, point_rect[1].x, point_rect[1].y, MaixColor(0, 255, 0), 1);
            libmaix_cv_image_draw_line(img, point_rect[2].x, point_rect[2].y, point_rect[1].x, point_rect[1].y, MaixColor(0, 255, 0), 1);
            libmaix_cv_image_draw_line(img, point_rect[2].x, point_rect[2].y, point_rect[3].x, point_rect[3].y, MaixColor(0, 255, 0), 1);
            libmaix_cv_image_draw_line(img, point_rect[3].x, point_rect[3].y, point_rect[1].x, point_rect[1].y, MaixColor(0, 255, 0), 1);

            /* do something useful with results */
            zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
            const char *data = zbar_symbol_get_data(symbol);
            if (typ == ZBAR_QRCODE) {
                int datalen = strlen(data);
                libmaix_cv_image_draw_string(img, 0, 0, zbar_get_symbol_name(typ), 1.5, MaixColor(0, 0, 255), 1);
                libmaix_cv_image_draw_string(img, 0, 20, data, 1.5, MaixColor(255, 0, 0), 1);
                printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);
                // break;
            }
        }

        /* clean up */
        zbar_image_destroy(image); // use zbar_image_free_data
    }

}

void test_work() {

  test.cam0->start_capture(test.cam0);

  #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
  test.cam1->start_capture(test.cam1);
  #endif

  while (test.is_run)
  {
    // goal code
    libmaix_image_t *tmp = NULL;
    if (LIBMAIX_ERR_NONE == test.cam0->capture_image(test.cam0, &tmp))
    {
        // printf("w %d h %d p %d \r\n", tmp->width, tmp->height, tmp->mode);
        // qrcode_loop(tmp);
        apriltag_loop(tmp);

        if (tmp->width == test.disp->width && test.disp->height == tmp->height) {
            test.disp->draw_image(test.disp, tmp);
        } else {
            libmaix_image_t *rs = libmaix_image_create(test.disp->width, test.disp->height, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
            if (rs) {
                libmaix_cv_image_resize(tmp, test.disp->width, test.disp->height, &rs);
                test.disp->draw_image(test.disp, rs);
                libmaix_image_destroy(&rs);
            }
        }
        CALC_FPS("maix_cam 0");

        #ifdef CONFIG_ARCH_V831 // CONFIG_ARCH_V831 & CONFIG_ARCH_V833
        libmaix_image_t *t = NULL;
        if (LIBMAIX_ERR_NONE == test.cam1->capture_image(test.cam1, &t))
        {
            // printf("w %d h %d p %d \r\n", t->width, t->height, t->mode);
            CALC_FPS("maix_cam 1");
        }
        #endif
    }
  }

}

int main(int argc, char **argv)
{
  signal(SIGINT, test_handlesig);
  signal(SIGTERM, test_handlesig);

  libmaix_image_module_init();

  test_init();
//   qrcode_init();
  apriltag_init();
  test_work();
  apriltag_exit();
//   qrcode_exit();
  test_exit();


  libmaix_image_module_deinit();

  return 0;
}
