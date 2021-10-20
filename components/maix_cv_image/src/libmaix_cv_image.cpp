
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/types_c.h"

#include "libmaix_image.h"

// class libmaix_cv_image
// {
// private:
//     /* data */
// public:
//     libmaix_cv_image(/* args */);
//     ~libmaix_cv_image();
// };

// libmaix_cv_image::libmaix_cv_image(/* args */)
// {
// }

// libmaix_cv_image::~libmaix_cv_image(){};

extern "C"
{

    libmaix_err_t libmaix_cv_image_test(libmaix_image_t *src, libmaix_image_t *dst)
    {
        if (dst->data == NULL && src->data == NULL && src->mode != dst->mode) {
            return LIBMAIX_ERR_PARAM;
        }

        if (src->mode != LIBMAIX_IMAGE_MODE_RGB888) {
            return LIBMAIX_ERR_NOT_IMPLEMENT;
        }

        cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));

        cv::rectangle(input, cv::Rect(170, 50, 50, 50), cv::Scalar(0, 255, 0), 4);

        cv::putText(input,"raw video", cv::Point(50, 30), 2, 1.0f, CV_RGB(255,0,0));

        memcpy(dst->data, input.data, src->width * src->height * 3);

		return LIBMAIX_ERR_NONE;
    }

    void libmaix_cv_image_draw_circle()
    {

    }
}