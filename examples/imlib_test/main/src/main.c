
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include "time.h"

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "libmaix_cam.h"
#include "libmaix_image.h"
#include "libmaix_disp.h"
#include "imlib.h"
#include "libmaix_cv_image.h"
#include <sys/time.h>

#define CALC_FPS(tips)                                                                                         \
	{                                                                                                          \
		static int fcnt = 0;                                                                                   \
		fcnt++;                                                                                                \
		static struct timespec ts1, ts2;                                                                       \
		clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
		if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
		{                                                                                                      \
			printf("%s => H26X FPS:%d\n", tips, fcnt);                                                         \
			ts1 = ts2;                                                                                         \
			fcnt = 0;                                                                                          \
		}                                                                                                      \
	}

static struct timeval old, now;

void cap_set()
{
  gettimeofday(&old, NULL);
}

void cap_get(const char *tips)
{
  gettimeofday(&now, NULL);
  if (now.tv_usec > old.tv_usec)
    printf("%20s - %5ld us\r\n", tips, (now.tv_usec - old.tv_usec));
}

// run flage
int run_flage = 1;

// ctrl + c exit signal handle!
void signal_handle(int signal_num)
{
	run_flage = 0;
}

#ifdef CONFIG_ARCH_V831 // V831 platform

static struct libmaix_cam *cam = NULL;
static struct libmaix_cam *cam1 = NULL;
static struct libmaix_disp *disp = NULL;
static libmaix_image_t *img = NULL;
static int width = 240, height = 240;

void test_init()
{
	// module init
	imlib_init_all();

	libmaix_image_module_init();
	libmaix_camera_module_init();

	// create camera interface， open two cameras, this is a must, v831 platform feature！
	cam = libmaix_cam_create(0, width, height, 1, 0);
	assert(NULL != cam);
	cam->start_capture(cam);

	cam1 = libmaix_cam_create(1, width, height, 0, 0);
	assert(NULL != cam1);

	// create disp interface
	disp = libmaix_disp_create(0);
	assert(NULL != disp);
}
void test_exit()
{
	// free interface
	libmaix_disp_destroy(&disp), disp = NULL;
	libmaix_cam_destroy(&cam1), cam1 = NULL;
	libmaix_cam_destroy(&cam), cam = NULL;

	// deinit module
	libmaix_camera_module_deinit();
	libmaix_image_module_deinit();

	imlib_deinit_all();
}

#else // other platform

static struct libmaix_cam *cam = NULL;
static struct libmaix_disp *disp = NULL;
static libmaix_image_t *img;
static int width = 240, height = 240;

void test_init()
{
	puts("imlib_init_all");
	imlib_init_all();

	libmaix_image_module_init();
	libmaix_camera_module_init();

	cam = libmaix_cam_create(0, width, height, 1, 0);
	assert(NULL != cam);
	cam->start_capture(cam);

	disp = libmaix_disp_create(0);
	assert(NULL != disp);
}
void test_exit()
{
	libmaix_disp_destroy(&disp), disp = NULL;
	libmaix_cam_destroy(&cam), cam = NULL;

	libmaix_camera_module_deinit();
	libmaix_image_module_deinit();

	imlib_deinit_all();
	puts("imlib_deinit_all");
}
#endif

void find_blobs(image_t *img_ts);
void find_lines(image_t *img_ts);
void find_cricle(image_t *img_ts);

void test_even()
{
	// Detect running flags
	while (run_flage)
	{
		// Get the picture, and determine whether the picture is normal
		int ret = cam->capture_image(cam, &img);
		// printf("ret %d\r\n", ret);
		if (LIBMAIX_ERR_NONE == ret)
		{
			// image_t *imlib_img = MAIX_2_IMLIB(img);

			image_t *imlib_img = imlib_image_create(img->width, img->height, PIXFORMAT_RGB888, img->width * img->height * PIXFORMAT_BPP_RGB888, img->data, false);

			//show imlib_image info
			imlib_printf_image_info(imlib_img);

			// draw test line on image

			// Statistical frame rate
			CALC_FPS("nihao");

			// imlib_draw_line(imlib_img, 10, 10, 10, 100,        COLOR_R8_G8_B8_TO_RGB888(0xff, 0x00, 0x00), 4);
			// imlib_draw_line(imlib_img, 20, 10, 20, 100,        COLOR_R8_G8_B8_TO_RGB888(0x00, 0xff, 0x00), 4);
			// imlib_draw_line(imlib_img, 30, 10, 30, 100,        COLOR_R8_G8_B8_TO_RGB888(0x00, 0x00, 0xff), 4);
			// imlib_draw_rectangle(imlib_img, 10, 120, 14, 100,  COLOR_R8_G8_B8_TO_RGB888(0xff, 0x00, 0x00), 2, 0);
			// imlib_draw_rectangle(imlib_img, 26, 120, 14, 100,  COLOR_R8_G8_B8_TO_RGB888(0x00, 0x00, 0xff), 1, 1);
			// imlib_draw_circle(imlib_img, 70, 30, 20,           COLOR_R8_G8_B8_TO_RGB888(0x00, 0x00, 0xff), 2, false);
			// imlib_draw_circle(imlib_img, 70, 100, 20,          COLOR_R8_G8_B8_TO_RGB888(0x00, 0x00, 0xff), 2, true);
			{
				// imlib_image_resize & save
				int w = 255, h = 255;
				image_t *resize_img = imlib_image_create(w, h, PIXFORMAT_RGB888, w * h * PIXFORMAT_BPP_RGB888, NULL, true);
				cap_set();
				imlib_image_resize(resize_img, imlib_img, IMAGE_HINT_AREA); // IMAGE_HINT_BILINEAR
				cap_get("imlib_image_resize");

				libmaix_image_t *libmaix_img = libmaix_image_create(resize_img->w, resize_img->h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, resize_img->data, false);

				libmaix_cv_image_save_file(libmaix_img, "./imlib_image_resize.jpg");

				const int ZOOM_AMOUNT = 1; // Lower zooms out - Higher zooms in 较低的值缩小-较高的放大
				const int FOV_WINDOW = 60; // Between 0 and 180. Represents the field-of-view of the scene
                // window when rotating the image in 3D space. When closer to
                // zero results in lines becoming straighter as the window
                // moves away from the image being rotated in 3D space. A large
                // value moves the window closer to the image in 3D space which
                // results in the more perspective distortion and sometimes
                // the image in 3D intersecting the scene window.
                // 在0和180之间。表示在三维空间中旋转图像时场景窗口的视场。
                // 当接近于0时，随着窗口远离在三维空间中旋转的图像，直线会变得更直。
                // 在三维空间中，较大的值会使窗口更靠近图像，从而导致更多的透视畸变，
                // 有时会导致三维图像与场景窗口相交。
				imlib_rotation_corr(resize_img, 0.3, 0, 0, 0, 0, ZOOM_AMOUNT, FOV_WINDOW, NULL);
			
				libmaix_cv_image_save_file(libmaix_img, "./imlib_rotation_corr.jpg");

				libmaix_image_t *libmaix_tmp = libmaix_image_create(255, 255, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
				if (libmaix_tmp) {
					cap_set();
					libmaix_cv_image_resize(img, libmaix_tmp->width, libmaix_tmp->height, &libmaix_tmp);
					cap_get("libmaix_cv_image_resize");
					libmaix_cv_image_save_file(libmaix_tmp, "./libmaix_cv_image_resize.jpg");
					libmaix_image_destroy(&libmaix_tmp);
				}

				libmaix_image_destroy(&libmaix_img);

				imlib_image_destroy(&resize_img);
			}
			// find_blobs(imlib_img);
			find_lines(imlib_img);
			// find_cricle(imlib_img);

			// display image
			disp->draw_image(disp, img);

			// free imlib image
			imlib_image_destroy(&imlib_img);
		}
		else
		{
			// It will exit when there is an error in getting the image
			break;
		}
	}
}

void find_blobs(image_t *img_ts)
{
	image_t *arg_img = img_ts;

	list_t thresholds;
	imlib_list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));

	color_thresholds_list_lnk_data_t tmp_ct;
	tmp_ct.LMin = 44;
	tmp_ct.AMin = 10;
	tmp_ct.BMin = -49;
	tmp_ct.LMax = 80;
	tmp_ct.AMax = 110;
	tmp_ct.BMax = 115;
	list_push_back(&thresholds, &tmp_ct);

	bool invert = false;

	rectangle_t roi;
	roi.x = 0;
	roi.y = 0;
	roi.w = 240;
	roi.h = 240;

	unsigned int x_stride = 10;
	unsigned int y_stride = 10;

	unsigned int area_threshold = 50;
	unsigned int pixels_threshold = 50;
	bool merge = false;
	int margin = 0;
	unsigned int x_hist_bins_max = 0;
	unsigned int y_hist_bins_max = 0;

	list_t out;
	fb_alloc_mark();

	imlib_find_blobs(&out, arg_img, &roi, x_stride, y_stride, &thresholds, invert,
					 area_threshold, pixels_threshold, merge, margin,
					 NULL, NULL, NULL, NULL, x_hist_bins_max, y_hist_bins_max);
	fb_alloc_free_till_mark();

	list_free(&thresholds);

	for (size_t i = 0; list_size(&out); i++)
	{
		find_blobs_list_lnk_data_t lnk_data;
		list_pop_front(&out, &lnk_data);
		printf("find ones!\n");
		switch (arg_img->pixfmt)
		{
		case PIXFORMAT_RGB565:
			imlib_draw_rectangle(img_ts, lnk_data.rect.x, lnk_data.rect.y, lnk_data.rect.w, lnk_data.rect.h, COLOR_R8_G8_B8_TO_RGB888(0x00, 0x00, 0x00), 1, 0);
			break;
		case PIXFORMAT_RGB888:
			imlib_draw_rectangle(img_ts, lnk_data.rect.x, lnk_data.rect.y, lnk_data.rect.w, lnk_data.rect.h, COLOR_R8_G8_B8_TO_RGB565(0x00, 0x00, 0x00), 1, 0);
			break;
		default:
			break;
		}
	}
}
void find_lines(image_t *img_ts)
{
	image_t *arg_img = img_ts;

	rectangle_t roi;
	roi.x = 0;
	roi.y = 0;
	roi.w = 240;
	roi.h = 240;

	unsigned int x_stride = 2;
	unsigned int y_stride = 1;
	uint32_t threshold = 1000;
	unsigned int theta_margin = 25;
	unsigned int rho_margin = 25;

	list_t out_;
	fb_alloc_mark();
	imlib_find_lines(&out_, arg_img, &roi, x_stride, y_stride, threshold, theta_margin, rho_margin);
	fb_alloc_free_till_mark();

	for (size_t i = 0; list_size(&out_); i++)
	{
		find_lines_list_lnk_data_t lnk_data;
		list_pop_front(&out_, &lnk_data);
		printf("fine line ones!\n");

		switch (arg_img->pixfmt)
		{
		case PIXFORMAT_RGB565:
			imlib_draw_line(img_ts, lnk_data.line.x1, lnk_data.line.y1, lnk_data.line.x2, lnk_data.line.y2, COLOR_R8_G8_B8_TO_RGB888(0x00, 0xff, 0x00), 1);
			break;
		case PIXFORMAT_RGB888:
			imlib_draw_line(img_ts, lnk_data.line.x1, lnk_data.line.y1, lnk_data.line.x2, lnk_data.line.y2, COLOR_R8_G8_B8_TO_RGB565(0x00, 0xff, 0x00), 1);
			break;
		default:
			break;
		}
	}
}

void find_cricle(image_t *img_ts)
{
	image_t *arg_img = img_ts;

	rectangle_t roi;
	roi.x = 0;
	roi.y = 0;
	roi.w = 240;
	roi.h = 240;

	unsigned int x_stride = 2;

	unsigned int y_stride = 1;

	uint32_t threshold = 3000;
	unsigned int x_margin = 10;
	unsigned int y_margin = 10;
	unsigned int r_margin = 10;
	unsigned int r_min = 15;
	unsigned int r_max = 25;
	unsigned int r_step = 2;

	list_t out;
	fb_alloc_mark();
	imlib_find_circles(&out, arg_img, &roi, x_stride, y_stride, threshold, x_margin, y_margin, r_margin,
					   r_min, r_max, r_step);
	fb_alloc_free_till_mark();

	for (size_t i = 0; list_size(&out); i++)
	{
		find_circles_list_lnk_data_t lnk_data;
		list_pop_front(&out, &lnk_data);
		printf("find_cricles!\n");

		switch (arg_img->pixfmt)
		{
		case PIXFORMAT_RGB565:
			imlib_draw_circle(img_ts, 70, 30, 20, COLOR_R8_G8_B8_TO_RGB888(0x00, 0xff, 0x00), 1, false);
			break;
		case PIXFORMAT_RGB888:
			imlib_draw_circle(img_ts, lnk_data.p.x, lnk_data.p.y, lnk_data.r, COLOR_R8_G8_B8_TO_RGB565(0x00, 0xff, 0x00), 1, false);
			break;
		default:
			break;
		}
	}
}

int main(int argc, char **argv)
{
	signal(SIGINT, signal_handle);
	test_init();
	test_even();
	test_exit();
	exit(0);
	return 0;
}
