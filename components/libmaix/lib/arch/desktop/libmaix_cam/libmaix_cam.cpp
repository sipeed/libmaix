#include "libmaix_cam.h"

#include "libmaix_cam_priv.h"

#include <cstring>

#include <opencv2/opencv.hpp>
#include "opencv2/core/types_c.h"

extern "C" {

void libmaix_camera_module_init()
{
    return ;
}

void libmaix_camera_module_deinit()
{
    return ;
}

int vi_init_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    char dev_name[32] = "";
    sprintf(dev_name, "/dev/video%d", priv->vi_dev);
    priv->vcap = new V4L2Capture(dev_name, priv->vi_w, priv->vi_h);
    priv->vcap->openDevice();
    priv->vcap->initDevice();
    priv->vi_img = NULL;
    priv->inited = 1;
    return 0;
}

int vi_deinit_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited)
    {
        priv->inited = 0;
        priv->vcap->freeBuffers();
        priv->vcap->closeDevice();
    }
    return priv->inited;
}

libmaix_err_t vi_start_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited)
    {
        priv->vcap->startCapture();
        // cam->width = priv->vcap->capW, cam->height = priv->vcap->capH; // need resize & cut
        cam->fram_size = (cam->width * cam->height * 3);
        if (priv->vi_w < priv->vcap->capW) priv->vi_x = (priv->vcap->capW - priv->vi_w) / 2;
        if (priv->vi_h < priv->vcap->capH) priv->vi_y = (priv->vcap->capH - priv->vi_h) / 2;
        return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_READY;
}

libmaix_err_t vi_priv_capture_image(struct libmaix_cam *cam, struct libmaix_image **img)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->vi_img == NULL) {
      priv->vi_img = libmaix_image_create(priv->vi_w, priv->vi_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
      if(!priv->vi_img) return LIBMAIX_ERR_NO_MEM;
    }

    if (priv->inited) {
        unsigned char *yuv422frame = NULL;
        size_t readlen = 0;
        if (0 == priv->vcap->getFrame((void **)&yuv422frame, (size_t *)&readlen)) {
            cv::Mat src(priv->vcap->capH, priv->vcap->capW, CV_8UC2, (void*)yuv422frame);
            // cv::imwrite("src.jpg", src);
            const int cmp[2][2] = { {2, 1}, {0, -1}, };
            int flip_code = cmp[priv->vi_m][priv->vi_f];
            if (flip_code != 2) cv::flip(src, src, flip_code);
            // printf("[vi_priv_capture] %d %d %d\r\n", priv->vi_m, priv->vi_f, flip_code);

            // cv::imwrite("dst.jpg", dst);
            if (priv->vi_w != priv->vcap->capH || priv->vi_h != priv->vcap->capW) {
              cv::Mat dst;
              cv::cvtColor(src, dst, (priv->vi_f == 1) ? cv::COLOR_YUV2RGB_YUYV : cv::COLOR_YUV2BGR_YUYV); // opencv4 flip yuv bug
              cv::Mat tmp(priv->vi_h, priv->vi_w, CV_8UC3, priv->vi_img->data);
              dst(cv::Rect(priv->vi_x, priv->vi_y, priv->vi_w, priv->vi_h)).copyTo(tmp);
              // cv::imwrite("tmp.jpg", tmp);
              // printf("[vi_priv_capture] %d %d %d %d\r\n", t.cols, t.rows, priv->vcap->capW, priv->vcap->capH);
            } else {
              cv::Mat dst(priv->vi_h, priv->vi_w, CV_8UC3, priv->vi_img->data);
              cv::cvtColor(src, dst, (priv->vi_f == 1) ? cv::COLOR_YUV2RGB_YUYV : cv::COLOR_YUV2BGR_YUYV); // opencv4 flip yuv bug
            }
            priv->vcap->backFrame();
            *img = priv->vi_img;
            return LIBMAIX_ERR_NONE;
        }
    }
    return LIBMAIX_ERR_NOT_READY;
}

libmaix_err_t vi_priv_capture(struct libmaix_cam *cam, unsigned char *buf)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited) {
        unsigned char *yuv422frame = NULL;
        size_t readlen = 0;
        if (0 == priv->vcap->getFrame((void **)&yuv422frame, (size_t *)&readlen)) {
            cv::Mat src(priv->vcap->capH, priv->vcap->capW, CV_8UC2, (void*)yuv422frame);
            // cv::imwrite("src.jpg", src);
            cv::Mat dst;
            cv::cvtColor(src, dst, cv::COLOR_YUV2RGB_YUYV);

            // monkey patch 2021
            cv::Mat patch;
            cv::flip(dst, patch, 0); // cv::rotate(dst, patch, cv::ROTATE_180);
            dst = patch;
            // patch end

            // cv::imwrite("dst.jpg", dst);
            if (cam->fram_size != dst.total() * dst.elemSize()) {
              cv::Mat tmp;
              dst(cv::Rect(priv->vi_x, priv->vi_y, priv->vi_w, priv->vi_h)).copyTo(tmp);
              // cv::imwrite("tmp.jpg", tmp);
              // printf("[vi_priv_capture] %d %d %d %d\r\n", t.cols, t.rows, priv->vcap->capW, priv->vcap->capH);
              memcpy(buf, tmp.data, cam->fram_size);
            } else {
              memcpy(buf, dst.data, cam->fram_size);
            }
            priv->vcap->backFrame();
            return LIBMAIX_ERR_NONE;
        }
    }
    return LIBMAIX_ERR_NOT_READY;
}

int cam_priv_init(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t*)cam->reserved;

    if(NULL == priv) {
        fprintf(stderr, "cam: priv is NULL\n");
        return -1;
    }

    // sipeed v833 0 1 to mpp, 2 3 to dvp csi1 sensor use camerademo(v4l2)
    // sipeed r329 only have v4l2

    priv->devInit = vi_init_capture;
    priv->devDeinit = vi_deinit_capture;

    cam->start_capture = vi_start_capture;
    cam->capture = vi_priv_capture;
    cam->capture_image = vi_priv_capture_image;

    return priv->devInit(cam);
}


struct libmaix_cam * libmaix_cam_create(int n, int w, int h, int m, int f)
{
    struct libmaix_cam *cam = (struct libmaix_cam*)malloc(sizeof(struct libmaix_cam));
    if(NULL == cam) {
        return NULL;
    }

    cam->width = w;
    cam->height = h;

    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t *)malloc(sizeof(struct libmaix_cam_priv_t));
    if(NULL == priv) {
        free(cam);
        return NULL;
    }

    memset(priv, 0, sizeof(struct libmaix_cam_priv_t));
    cam->reserved = (void*)priv;

    priv->vi_dev = n;
    priv->vi_x = 0;
    priv->vi_y = 0;
    priv->vi_w = w;
    priv->vi_h = h;
    priv->vi_m = m;
    priv->vi_f = f;
    priv->inited = 0;

    if(cam_priv_init(cam) != 0) {
        libmaix_cam_destroy(&cam);
        return NULL;
    }

    return cam;
}

void libmaix_cam_destroy(struct libmaix_cam **cam)
{
    if(NULL == cam || NULL == *cam)
        return;

    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t*)(*cam)->reserved;

    if(priv) {
        if(priv->vi_img != NULL) {
            libmaix_image_destroy(&priv->vi_img);
        }
        if(priv->devDeinit) {
            priv->devDeinit(*cam);
        }
        free(priv);
    }

    free(*cam);
    *cam = NULL;
}

int libmaix_cam_unit_test(char * tmp)
{
  std::cout << time(NULL) << std::endl;
  cv::Mat frame;
  std::cout << time(NULL) << std::endl;
  cv::VideoCapture cap;
  std::cout << time(NULL) << std::endl;
  cap.open(0);
  std::cout << time(NULL) << std::endl;
  if (frame.empty())
    std::cout << "frame.empty()" << std::endl; // Ran out of film
  std::cout << time(NULL) << std::endl;

  while (1) {
    cap >> frame;
    if (!frame.empty()) {
      std::cout << time(NULL) << std::endl;
    }
    // fps 25
  }

  return 0;
}

// void rgb888_to_rgb565(uint8_t * rgb888, uint16_t width, uint16_t height, uint16_t * rgb565)
// {
//     cv::Mat tmp(width, height, CV_8UC3, (void*)rgb888);
//     // cv::imwrite("tmp.jpg", tmp);
//     cv::Mat img;
//     cv::cvtColor(tmp, img, cv::COLOR_RGB2BGR565);
//     memcpy(rgb565, img.data, width * height * 2);
// }

}
