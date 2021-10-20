
#include "libmaix_image.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/types_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/freetype.hpp>

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
        if (dst->data == NULL && src->data == NULL && src->mode != dst->mode)
        {
            return LIBMAIX_ERR_PARAM;
        }

        if (src->mode != LIBMAIX_IMAGE_MODE_RGB888)
        {
            return LIBMAIX_ERR_NOT_IMPLEMENT;
        }

        cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));

        cv::rectangle(input, cv::Rect(170, 50, 50, 50), cv::Scalar(0, 255, 0), 4);

        cv::putText(input, "abcdefg", cv::Point(50, 30), 2, 1.0f, CV_RGB(255, 0, 0));

        cv::String text = u8"123测试asd的テスター";
        int fontHeight = 20;
        int thickness = -1;
        int linestyle = 8;
        int baseline = 0;

        cv::Ptr<cv::freetype::FreeType2> ft2;
        ft2 = cv::freetype::createFreeType2();
        ft2->loadFontData("./txwzs.ttf", 0);

        cv::Size textSize = ft2->getTextSize(text,
                                         fontHeight,
                                         thickness,
                                         &baseline);

        if (thickness > 0)
        {
            baseline += thickness;
        }

        cv::Point textOrg((input.cols - textSize.width) / 3, (input.rows + textSize.height) / 3);

        cv::rectangle(input, textOrg + cv::Point(0, baseline),
                  textOrg + cv::Point(textSize.width, -textSize.height),
                  cv::Scalar(0, 255, 0), 1, 8);

        cv::line(input, textOrg + cv::Point(0, thickness),
             textOrg + cv::Point(textSize.width, thickness),
             cv::Scalar(0, 0, 255), 1, 8);

        ft2->putText(input, text, textOrg, fontHeight,
                     cv::Scalar::all(255), thickness, linestyle, true);

        memcpy(dst->data, input.data, src->width * src->height * 3);

        return LIBMAIX_ERR_NONE;
    }
}
