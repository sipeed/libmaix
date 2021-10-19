
#include <opencv2/opencv.hpp>
#include "opencv2/core/types_c.h"

// #include "libmaix_image.h"

class libmaix_cv_image
{
private:
    /* data */
public:
    libmaix_cv_image(/* args */);
    ~libmaix_cv_image();
};

libmaix_cv_image::libmaix_cv_image(/* args */)
{
}

libmaix_cv_image::~libmaix_cv_image(){};

extern "C"
{

#include "stdio.h"

    void libmaix_cv_image_test()
    {
        auto t = libmaix_cv_image();

        cv::Mat dst;

        // monkey patch 2021
        cv::Mat patch;
        cv::flip(dst, patch, 0); // cv::rotate(dst, patch, cv::ROTATE_180);
        dst = patch;

        puts("libmaix_cv_image_cpp");
    }
}