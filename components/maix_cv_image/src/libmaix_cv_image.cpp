
#include "libmaix_cv_image.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
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

void overlayImage(const cv::Mat &background, const cv::Mat &foreground,
                  cv::Mat &output, cv::Point2i location)
{
    background.copyTo(output);
    // start at the row indicated by location, or at row 0 if location.y is negative.
    for (int y = std::max(location.y, 0); y < background.rows; ++y)
    {
        int fY = y - location.y; // because of the translation
        // we are done of we have processed all rows of the foreground image.
        if (fY >= foreground.rows)
            break;

        // start at the column indicated by location,
        // or at column 0 if location.x is negative.
        for (int x = std::max(location.x, 0); x < background.cols; ++x)
        {
            int fX = x - location.x; // because of the translation.
            // we are done with this row if the column is outside of the foreground image.
            if (fX >= foreground.cols)
                break;

            // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
            double opacity =
                ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.;

            // and now combine the background and foreground pixel, using the opacity,
            // but only if opacity > 0.
            for (int c = 0; opacity > 0 && c < output.channels(); ++c)
            {
                unsigned char foregroundPx =
                    foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                    background.data[y * background.step + x * background.channels() + c];
                output.data[y * output.step + output.channels() * x + c] =
                    backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
}

//图片混合
bool mergeImage(cv::Mat &srcImage, cv::Mat mixImage, cv::Point startPoint)
{
    //检查图片数据
    if (!srcImage.data || !mixImage.data)
    {
        // cout << "输入图片 数据错误！" << endl ;
        return LIBMAIX_ERR_PARAM;
    }
    //检查行列是否越界
    int addCols = startPoint.x + mixImage.cols > srcImage.cols ? 0 : mixImage.cols;
    int addRows = startPoint.y + mixImage.rows > srcImage.rows ? 0 : mixImage.rows;
    if (addCols == 0 || addRows == 0)
    {
        // cout << "添加图片超出" << endl;
        return LIBMAIX_ERR_PARAM;
    }

    //ROI 混合区域
    cv::Mat roiImage = srcImage(cv::Rect(startPoint.x, startPoint.y, addCols, addRows));

    // printf("MixImage %d %d\r\n", srcImage.type() == CV_8UC3, mixImage.type() == CV_8UC4);

    //图片类型一致
    if (srcImage.type() == mixImage.type())
    {
        puts("copyTo");
        mixImage.copyTo(roiImage, mixImage);
        return LIBMAIX_ERR_NONE;
    }

    cv::Mat maskImage;

    //原始图片：RGB  贴图：ARGB
    if (srcImage.type() == CV_8UC3 && mixImage.type() == CV_8UC4)
    {
        cv::cvtColor(mixImage, maskImage, cv::ColorConversionCodes::COLOR_RGBA2BGR);
        maskImage.copyTo(roiImage, maskImage);
        return LIBMAIX_ERR_NONE;
    }

    //原始图片：灰度  贴图：彩色
    if (srcImage.type() == CV_8U && mixImage.type() == CV_8UC3)
    {
        cv::cvtColor(mixImage, maskImage, cv::ColorConversionCodes::COLOR_BGR2GRAY);
        maskImage.copyTo(roiImage, maskImage);
        return LIBMAIX_ERR_NONE;
    }

    //原始图片：彩色  贴图：灰色
    if (srcImage.type() == CV_8UC3 && mixImage.type() == CV_8U)
    {
        cv::cvtColor(mixImage, maskImage, cv::ColorConversionCodes::COLOR_GRAY2BGR);
        maskImage.copyTo(roiImage, maskImage);
        return LIBMAIX_ERR_NONE;
    }

    return LIBMAIX_ERR_NOT_IMPLEMENT;
}

extern "C"
{
    libmaix_err_t libmaix_cv_image_draw_ellipse(libmaix_image_t *src, int x, int y, int w, int h, double angle, double startAngle, double endAngle, libmaix_image_color_t color, int thickness)
    {
        if (src->data == NULL)
        {
            return LIBMAIX_ERR_PARAM;
        }
        if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
        {
            cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));
            cv::ellipse(input, cv::Point(x, y), cv::Size(w, h), angle, startAngle, endAngle, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
            memcpy(src->data, input.data, src->width * src->height * 3);
            return LIBMAIX_ERR_NONE;
        }
        return LIBMAIX_ERR_NOT_IMPLEMENT;
    }

    libmaix_err_t libmaix_cv_image_draw_circle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness)
    {
        if (src->data == NULL)
        {
            return LIBMAIX_ERR_PARAM;
        }
        if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
        {
            cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));
            cv::circle(input, cv::Point(x, y), r, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
            memcpy(src->data, input.data, src->width * src->height * 3);
            return LIBMAIX_ERR_NONE;
        }
        return LIBMAIX_ERR_NOT_IMPLEMENT;
    }

    libmaix_err_t libmaix_cv_image_draw_rectangle(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness)
    {
        if (src->data == NULL)
        {
            return LIBMAIX_ERR_PARAM;
        }
        if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
        {
            cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));
            cv::circle(input, cv::Point(x, y), r, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
            memcpy(src->data, input.data, src->width * src->height * 3);
            return LIBMAIX_ERR_NONE;
        }
        return LIBMAIX_ERR_NOT_IMPLEMENT;
    }

    libmaix_err_t libmaix_cv_image_draw_line(libmaix_image_t *src, int x, int y, int r, libmaix_image_color_t color, int thickness)
    {
        if (src->data == NULL)
        {
            return LIBMAIX_ERR_PARAM;
        }
        if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
        {
            cv::Mat input(src->width, src->height, CV_8UC3, const_cast<char *>((char *)src->data));
            cv::circle(input, cv::Point(x, y), r, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
            memcpy(src->data, input.data, src->width * src->height * 3);
            return LIBMAIX_ERR_NONE;
        }
        return LIBMAIX_ERR_NOT_IMPLEMENT;
    }

    libmaix_err_t libmaix_cv_image_draw(libmaix_image_t *src, libmaix_image_t *dst)
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

        cv::Mat image = cv::imread("/home/res/logo.png", CV_LOAD_IMAGE_UNCHANGED);

        if (!image.empty())
        {
            mergeImage(input, image, cv::Point(5, 3));
        }

        cv::rectangle(input, cv::Rect(170, 50, 50, 50), cv::Scalar(0, 255, 0), 4);

        cv::putText(input, "abcdefg", cv::Point(50, 30), 2, 1.0f, CV_RGB(255, 0, 0));

        // cv::String text = u8"123测试asd的テスター";

        // int fontHeight = 20;
        // int thickness = -1;
        // int linestyle = 8;
        // int baseline = 0;

        // cv::Ptr<cv::freetype::FreeType2> ft2;
        // ft2 = cv::freetype::createFreeType2();
        // ft2->loadFontData("./txwzs.ttf", 0);

        // cv::Size textSize = ft2->getTextSize(text,
        //                                      fontHeight,
        //                                      thickness,
        //                                      &baseline);

        // if (thickness > 0)
        // {
        //     baseline += thickness;
        // }

        // cv::Point textOrg((input.cols - textSize.width) * 0.75, (input.rows + textSize.height) * 0.75);

        // cv::line(input, textOrg + cv::Point(0, thickness + 5),
        //          textOrg + cv::Point(textSize.width, thickness + 50),
        //          cv::Scalar(0, 0, 255), 1, 8);

        // ft2->putText(input, text, textOrg, fontHeight,
        //              cv::Scalar(0, 255, 0), thickness, linestyle, true);

        int w = 240, h = 240, r = 40, a = 10, b = 20;
        cv::Point center(w / 2, h / 2);

        cv::circle(input, center, r, cv::Scalar(0, 0, 255), cv::FILLED);
        cv::ellipse(input, center, cv::Size(a, b), 0, 0, 360, cv::Scalar(255, 0, 0));
        cv::ellipse(input, center, cv::Size(a, b), 45, 0, 360, cv::Scalar(255, 0, 0));
        cv::ellipse(input, center, cv::Size(a, b), -45, 0, 360, cv::Scalar(255, 0, 0));
        cv::ellipse(input, center, cv::Size(a, b), 90, 0, 360, cv::Scalar(255, 0, 0));

        const int numPts = 4;
        cv::Point pts[numPts] = {cv::Point(10, 10), cv::Point(5, 30), cv::Point(35, 30), cv::Point(30, 10)};
        const cv::Point *ppt[1] = {pts};
        cv::fillPoly(input, ppt, &numPts, 1, cv::Scalar(0, 255, 0));

        memcpy(dst->data, input.data, src->width * src->height * 3);

        return LIBMAIX_ERR_NONE;
    }
}
