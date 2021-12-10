
#include "libmaix_cv_image.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include "opencv2/core/types_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/freetype.hpp>

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location, double opacity = -1.0)
{
  background.copyTo(output);

  // start at the row indicated by location, or at row 0 if location.y is negative.
  for (int y = std::max(location.y, 0); y < background.rows; ++y)
  {
    int fY = y - location.y; // because of the translation

    // we are done of we have processed all rows of the foreground image.
    if (fY >= foreground.rows)
      break;

    // start at the column indicated by location, or at column 0 if location.x is negative.
    for (int x = std::max(location.x, 0); x < background.cols; ++x)
    {
      int fX = x - location.x; // because of the translation.

      // we are done with this row if the column is outside of the foreground image.
      if (fX >= foreground.cols)
        break;

      double opacity_level = 0.;
      if (opacity < 0.) {
        opacity_level = 1.0;
      } else {
        // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
        opacity_level = ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.;
        if (opacity >= 0. && opacity < 1.)
          opacity_level *= opacity;
      }

      // and now combine the background and foreground pixel, using the opacity, but only if opacity > 0.
      for (int c = 0; opacity_level > 0 && c < output.channels(); ++c)
      {
        unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
        unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
        output.data[y * output.step + output.channels() * x + c] = backgroundPx * (1. - opacity_level) + foregroundPx * opacity_level;
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
    return false;
    // cout << "添加图片超出" << endl;
    // return LIBMAIX_ERR_PARAM;
    // mixImage = mixImage(cv::Rect(startPoint.x, startPoint.y, addCols - startPoint.x, addRows - startPoint.y));
  }

  // if (mixImage.type() != CV_8UC4) {
  //原图不是 ARGB 直接抹黑，不采用透明融合的方式
  // cv::rectangle(srcImage, startPoint, cv::Point(startPoint.x + mixImage.cols - 1, startPoint.y + mixImage.rows - 1), cv::Scalar(0, 0, 0), -1);
  // }

  //ROI 混合区域
  cv::Mat roiImage = srcImage(cv::Rect(startPoint.x, startPoint.y, addCols, addRows));

  // printf("MixImage %d %d\r\n", srcImage.type() == CV_8UC3, mixImage.type() == CV_8UC4);

  //图片类型一致
  if (srcImage.type() == mixImage.type())
  {
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

class libmaix_font
{
public:
  static cv::Ptr<cv::freetype::FreeType2> ft;
  static bool is_load;
};

cv::Ptr<cv::freetype::FreeType2> libmaix_font::ft = cv::freetype::createFreeType2();
bool libmaix_font::is_load = false;

extern "C"
{
#include "libmaix_debug.h"
#include <unistd.h>
#include <fcntl.h>
  libmaix_err_t libmaix_cv_image_open_file(libmaix_image_t **src, const char *path)
  {
    if (access(path, F_OK) < 0)
      return LIBMAIX_ERR_NOT_EXEC;
    if (access(path, R_OK) < 0)
      return LIBMAIX_ERR_NOT_EXEC;
    if (src != NULL)
    {
      libmaix_image_destroy(src);
    }
    cv::Mat image = cv::imread(path); // maybe need export
    cv::cvtColor(image, image, cv::ColorConversionCodes::COLOR_BGR2RGB);
    *src = libmaix_image_create(image.cols, image.rows, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
    memcpy((*src)->data, image.data, (*src)->width * (*src)->height * 3);
    // printf("libmaix_cv_image_open_file success\r\n");

    return LIBMAIX_ERR_NONE;
  }

  libmaix_err_t libmaix_cv_image_draw_ellipse(libmaix_image_t *src, int x, int y, int w, int h, double angle, double startAngle, double endAngle, libmaix_image_color_t color, int thickness)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat input(src->height, src->width, CV_8UC3, const_cast<char *>((char *)src->data));
      cv::ellipse(input, cv::Point(x, y), cv::Size(w, h), angle, startAngle, endAngle, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
      // memcpy(src->data, input.data, src->width * src->height * 3);
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
      cv::Mat input(src->height, src->width, CV_8UC3, src->data);
      cv::circle(input, cv::Point(x, y), r, cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
      // memcpy(src->data, input.data, src->width * src->height * 3);
      return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_cv_image_draw_rectangle(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat input(src->height, src->width, CV_8UC3, src->data);
      cv::rectangle(input, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
      // memcpy(src->data, input.data, src->width * src->height * 3);
      return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_cv_image_draw_line(libmaix_image_t *src, int x1, int y1, int x2, int y2, libmaix_image_color_t color, int thickness)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat input(src->height, src->width, CV_8UC3, src->data);
      cv::line(input, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
      // memcpy(src->data, input.data, src->width * src->height * 3);
      return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_cv_image_draw_image(libmaix_image_t *src, int x, int y, libmaix_image_t *dst, double opacity)
  {
    if (src->data == NULL || dst->data == NULL || src->data == dst->data)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888 && src->mode == dst->mode)
    {
      cv::Mat back(src->height, src->width, CV_8UC3, src->data);
      cv::Mat fore(dst->height, dst->width, CV_8UC3, dst->data);
      // mergeImage(input, paste, cv::Point(x, y));
      overlayImage(back, fore, back, cv::Point(x, y), opacity);

      // mixImage = mixImage(cv::Rect(startPoint.x, startPoint.y, addCols - startPoint.x, addRows - startPoint.y));

      // cv::Mat tmp;
      // dst(cv::Rect(priv->vi_x, priv->vi_y, priv->vi_w, priv->vi_h)).copyTo(tmp);

      // if (input.data != src->data) {
      //   memcpy(src->data, input.data, src->width * src->height * 3);
      // }
      return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_cv_image_draw_image_open(libmaix_image_t *src, int x, int y, const char *path, double opacity)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat back(src->height, src->width, CV_8UC3, src->data);
      cv::Mat fore = cv::imread(path, CV_LOAD_IMAGE_UNCHANGED); // maybe need export
      if (!fore.empty())
      {
        cv::cvtColor(fore, fore, cv::ColorConversionCodes::COLOR_BGR2RGB);
        overlayImage(back, fore, back, cv::Point(x, y), opacity);
        // mergeImage(input, image, cv::Point(x, y));
        // memcpy(src->data, input.data, src->width * src->height * 3);
        return LIBMAIX_ERR_NONE;
      }
      return LIBMAIX_ERR_NOT_READY;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }
  libmaix_err_t libmaix_cv_image_draw_image_save(libmaix_image_t *src, const char *path)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat input(src->height, src->width, CV_8UC3, const_cast<char *>((char *)src->data));
      cv::Mat save_img;
      cv::cvtColor(input, save_img, cv::ColorConversionCodes::COLOR_RGB2BGR);
      cv::imwrite(path, save_img);
      return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  libmaix_err_t libmaix_cv_image_load_freetype(const char *path)
  {
    if (libmaix_font::is_load)
      libmaix_font::ft = cv::freetype::createFreeType2(); // re-load clear it
    libmaix_font::ft->loadFontData(cv::String(path), 0);
    libmaix_font::is_load = true;
    return LIBMAIX_ERR_NONE;
  }

  // libmaix_err_t libmaix_cv_image_font_free()
  // {
  //     delete libmaix_font::ft;
  //     return LIBMAIX_ERR_NONE;
  // }

  void libmaix_cv_image_get_string_size(int *width, int *height, const char *str, double scale, int thickness)
  {
      cv::String text(str);
      if (!libmaix_font::is_load)
      {
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_PLAIN, scale, thickness, &baseline);
        int tmp = baseline - (scale * thickness);
        *width = textSize.width, *height = textSize.height + tmp;
        // printf("old textSize w %d h %d b %d\r\n", textSize.width, textSize.height, baseline);
      }
      else
      {
        int fontHeight = 14, baseline = 0; // tested sans.ttf
        cv::Size textSize = libmaix_font::ft->getTextSize(text, fontHeight, thickness, &baseline);
        *width = textSize.width, *height = fontHeight * scale;
        // printf("new textSize w %d h %d b %d\r\n", textSize.width, textSize.height, baseline);
      }
  }

  libmaix_err_t libmaix_cv_image_draw_string(libmaix_image_t *src, int x, int y, const char *str, double scale, libmaix_image_color_t color, int thickness)
  {
    if (src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat input(src->height, src->width, CV_8UC3, src->data);
      cv::String text(str);
      if (!libmaix_font::is_load)
      {
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_PLAIN, scale, thickness, &baseline);
        // printf("old textSize w %d h %d b %d\r\n", textSize.width, textSize.height, baseline);
        int tmp = baseline - (scale * thickness);
        cv::putText(input, text, cv::Point(x, y + textSize.height + tmp), cv::FONT_HERSHEY_PLAIN, scale,
            cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), thickness);
      }
      else
      {
        int fontHeight = 14, baseline = 0;
        cv::Size textSize = libmaix_font::ft->getTextSize(text, fontHeight, thickness, &baseline);
        // printf("new textSize w %d h %d b %d\r\n", textSize.width, textSize.height, baseline);
        libmaix_font::ft->putText(input, text, cv::Point(x, y), fontHeight * scale,
            cv::Scalar(color.rgb888.r, color.rgb888.g, color.rgb888.b), -1, 8, false);
      }
      return LIBMAIX_ERR_NOT_READY;
    }
    return LIBMAIX_ERR_NOT_IMPLEMENT;
  }

  static inline int libmaix_cv_image_load(struct libmaix_image *src, struct libmaix_image **dst)
  {
    int new_mem = 0;
    if ((*dst) == NULL)
    {
      *dst = libmaix_image_create(src->width, src->height, src->mode, src->layout, NULL, true);
      if (!(*dst))
      {
        return LIBMAIX_ERR_NO_MEM;
      }
      new_mem = 1;
    }
    else
    {
      if ((*dst)->data == NULL)
      {
        (*dst)->data = malloc(src->width * src->height * 3);
        if (!(*dst)->data)
        {
          return LIBMAIX_ERR_NO_MEM;
        }
        (*dst)->is_data_alloc = true;
        new_mem = 2;
      }
      (*dst)->layout = src->layout;
      (*dst)->width = src->width;
      (*dst)->height = src->height;
    }
    return new_mem;
  }

  static inline void libmaix_cv_image_free(int err, int new_mem, struct libmaix_image **dst)
  {
    if (err != LIBMAIX_ERR_NONE)
    {
      if (new_mem == 2)
      {
        free((*dst)->data);
        (*dst)->data = NULL;
      }
      else if (new_mem == 1)
      {
        libmaix_image_destroy(dst);
      }
    }
  }
  /*
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_INVALID   :    0
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_BINARY   :     1
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_GRAY   :       2
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_RGB888   :     3
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_RGB565   :     4
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_RGBA8888   :   5
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :      6
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :        7
LIBMAIX_IMAGE_MODE_INVALID -> LIBMAIX_IMAGE_MODE_BGR888   :     8
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_INVALID   :     256
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_BINARY   :      257
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_GRAY   :        258
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_RGB888   :      259
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_RGB565   :      260
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_RGBA8888   :    261
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :       262
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :         263
LIBMAIX_IMAGE_MODE_BINARY -> LIBMAIX_IMAGE_MODE_BGR888   :      264
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_INVALID   :       512
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_BINARY   :        513
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_GRAY   :  514
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_RGB888   :        515
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_RGB565   :        516
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_RGBA8888   :      517
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :         518
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :   519
LIBMAIX_IMAGE_MODE_GRAY -> LIBMAIX_IMAGE_MODE_BGR888   :        520
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_INVALID   :     768
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_BINARY   :      769
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_GRAY   :        770
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_RGB888   :      771
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_RGB565   :      772
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_RGBA8888   :    773
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :       774
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :         775
LIBMAIX_IMAGE_MODE_RGB888 -> LIBMAIX_IMAGE_MODE_BGR888   :      776
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_INVALID   :     1024
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_BINARY   :      1025
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_GRAY   :        1026
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_RGB888   :      1027
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_RGB565   :      1028
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_RGBA8888   :    1029
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :       1030
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :         1031
LIBMAIX_IMAGE_MODE_RGB565 -> LIBMAIX_IMAGE_MODE_BGR888   :      1032
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_INVALID   :   1280
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_BINARY   :    1281
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_GRAY   :      1282
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_RGB888   :    1283
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_RGB565   :    1284
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_RGBA8888   :  1285
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :     1286
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :       1287
LIBMAIX_IMAGE_MODE_RGBA8888 -> LIBMAIX_IMAGE_MODE_BGR888   :    1288
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_INVALID   :      1536
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_BINARY   :       1537
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_GRAY   :         1538
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_RGB888   :       1539
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_RGB565   :       1540
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_RGBA8888   :     1541
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :        1542
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :  1543
LIBMAIX_IMAGE_MODE_YUV420SP_NV21 -> LIBMAIX_IMAGE_MODE_BGR888   :       1544
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_INVALID   :        1792
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_BINARY   :         1793
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_GRAY   :   1794
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_RGB888   :         1795
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_RGB565   :         1796
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_RGBA8888   :       1797
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :  1798
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :    1799
LIBMAIX_IMAGE_MODE_YUV422_YUYV -> LIBMAIX_IMAGE_MODE_BGR888   :         1800
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_INVALID   :     2048
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_BINARY   :      2049
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_GRAY   :        2050
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_RGB888   :      2051
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_RGB565   :      2052
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_RGBA8888   :    2053
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_YUV420SP_NV21   :       2054
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_YUV422_YUYV   :         2055
LIBMAIX_IMAGE_MODE_BGR888 -> LIBMAIX_IMAGE_MODE_BGR888   :      2056
*/
  libmaix_err_t libmaix_cv_image_convert(struct libmaix_image *src, libmaix_image_mode_t mode, struct libmaix_image **dst)
  {
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    if (dst == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->width == 0 || src->height == 0 || src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (mode == src->mode)
    {
      switch (src->mode)
      {
      case LIBMAIX_IMAGE_MODE_GRAY:
        memcpy((*dst)->data, src->data, src->width * src->height);
        break;
      case LIBMAIX_IMAGE_MODE_RGB888:
        memcpy((*dst)->data, src->data, src->width * src->height * 3);
        break;
      case LIBMAIX_IMAGE_MODE_RGB565:
        memcpy((*dst)->data, src->data, src->width * src->height * 2);
        break;
      case LIBMAIX_IMAGE_MODE_RGBA8888:
        memcpy((*dst)->data, src->data, src->width * src->height * 4);
        break;
      case LIBMAIX_IMAGE_MODE_BGR888:
        memcpy((*dst)->data, src->data, src->width * src->height * 3);
        break;
      default:
        return LIBMAIX_ERR_PARAM;
        break;
      }
      return LIBMAIX_ERR_NONE;
    }
    // int new_mem = libmaix_cv_image_load(src, dst);
    // -------------------------------
    uint16_t mode_conver = 0;
    mode_conver = (uint8_t)src->mode << 8;
    mode_conver |= (uint8_t)mode;
    switch (mode_conver)
    {
    // case (403):             //RGB565 -> RGB888
    // {
    //     break;
    // }
    case (772): //RGB888 -> RGB565
    {
      if (src == *dst || src->width != (*dst)->width || src->height != (*dst)->height)
        return LIBMAIX_ERR_PARAM;
      uint8_t *rgb888 = (uint8_t *)src->data;
      uint16_t *rgb565 = (uint16_t *)(*dst)->data;
      for (uint16_t *end = rgb565 + src->width * src->height; rgb565 < end; rgb565 += 1, rgb888 += 3)
      {
        // *rgb565 = make16color(rgb888[0], rgb888[1], rgb888[2]);
        *rgb565 = ((((rgb888[0] >> 3) & 31) << 11) | (((rgb888[1] >> 2) & 63) << 5) | ((rgb888[2] >> 3) & 31));
      }
      (*dst)->mode = mode;
      break;
    }
    case (776): //RGB888 -> BGR888
    {
      // printf("libmaix_image_hal_convert src->mode %d mode %d \r\n", src->mode, mode);
      uint8_t *rgb = (uint8_t *)(src->data), *bgr = (uint8_t *)(*dst)->data;
      for (uint8_t *end = rgb + src->width * src->height * 3; rgb < end; rgb += 3, bgr += 3)
      {
        bgr[2] = rgb[0], bgr[1] = rgb[1], bgr[0] = rgb[2];
      }
      (*dst)->mode = mode;
      break;
    }
    case (770): //RGB888 -> GRAY
    {
      if (src == *dst || src->width != (*dst)->width || src->height != (*dst)->height)
        return LIBMAIX_ERR_PARAM;

      uint8_t *rgb888 = (uint8_t *)src->data;
      uint8_t *GARY = (uint8_t *)(*dst)->data;
      for (uint8_t *end = rgb888 + src->width * src->height * 3; rgb888 < end; GARY += 1, rgb888 += 3)
      {
        *GARY = (rgb888[0] * 30 + rgb888[1] * 59 + rgb888[2] * 11 + 50) / 100;
      }
      break;
    }
    case (515): //GRAY -> RGB888
    {
      if (src == *dst || src->width != (*dst)->width || src->height != (*dst)->height)
        return LIBMAIX_ERR_PARAM;
      uint8_t *GARY = (uint8_t *)src->data;
      uint8_t *rgb888 = (uint8_t *)(*dst)->data;
      for (uint8_t *end = GARY + src->width * src->height; GARY < end; GARY += 1, rgb888 += 3)
      {
        rgb888[0] = *GARY;
        rgb888[1] = *GARY;
        rgb888[2] = *GARY;
      }
      break;
    }
    case (516): //GRAY -> RGB565
    {
      if (src == *dst || src->width != (*dst)->width || src->height != (*dst)->height)
        return LIBMAIX_ERR_PARAM;
      uint8_t *GARY = (uint8_t *)src->data;
      uint16_t *rgb565 = (uint16_t *)(*dst)->data;
      for (uint8_t *end = GARY + src->width * src->height; GARY < end; rgb565 += 1, GARY += 1)
      {
        *rgb565 = ((*GARY & 0xf8) << 11) | ((*GARY & 0xfC) << 5) | (*GARY & 0xf8);
      }
      break;
    }
    default:
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
      break;
    }
    // libmaix_cv_image_free(err, new_mem, dst);

    return err;
  }

  libmaix_err_t libmaix_cv_image_resize(struct libmaix_image *src, int w, int h, struct libmaix_image **dst)
  {
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    if (dst == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->width == 0 || src->height == 0 || src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    // int new_mem = libmaix_cv_image_load(src, dst);
    // -------------------------------
    switch (src->mode)
    {
    case LIBMAIX_IMAGE_MODE_RGB888:
    {
      if ((src->width == (*dst)->width) && (src->height == (*dst)->height))
      {
        memcpy((*dst)->data, src->data, src->width * src->height * 3);
        return LIBMAIX_ERR_NONE;
      }
      cv::Mat cv_src(src->height, src->width, CV_8UC3, src->data);
      cv::Mat dist;
      cv::resize(cv_src, dist, cv::Size(w, h));
      memcpy((*dst)->data, dist.data, w * h * 3);
      // (*dst)->width = w;
      // (*dst)->height = h;
      // (*dst)->mode = src->mode;
      return LIBMAIX_ERR_NONE;
    }
    break;
    case LIBMAIX_IMAGE_MODE_RGBA8888:
    {
      if ((src->width == (*dst)->width) && (src->height == (*dst)->height))
      {
        memcpy((*dst)->data, src->data, src->width * src->height * 4);
        return LIBMAIX_ERR_NONE;
      }
      cv::Mat cv_src(src->height, src->width, CV_8UC4, src->data);
      cv::Mat dist;
      cv::resize(cv_src, dist, cv::Size(w, h));
      memcpy((*dst)->data, dist.data, w * h * 3);
      // (*dst)->width = w;
      // (*dst)->height = h;
      // (*dst)->mode = src->mode;
      return LIBMAIX_ERR_NONE;
    }
    break;
    case LIBMAIX_IMAGE_MODE_GRAY:
    {
      if ((src->width == (*dst)->width) && (src->height == (*dst)->height))
      {
        memcpy((*dst)->data, src->data, src->width * src->height);
        return LIBMAIX_ERR_NONE;
      }
      cv::Mat cv_src(src->height, src->width, CV_8UC1, src->data);
      cv::Mat dist;
      cv::resize(cv_src, dist, cv::Size(w, h));
      memcpy((*dst)->data, dist.data, w * h);
      // (*dst)->width = w;
      // (*dst)->height = h;
      // (*dst)->mode = src->mode;
      return LIBMAIX_ERR_NONE;
    }
    break;
    default:
    {
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
      // libmaix_cv_image_free(err, new_mem, dst);
      return LIBMAIX_ERR_NOT_EXEC;
    }
    break;
    }
    // -------------------------------
    // libmaix_cv_image_free(err, new_mem, dst);
    return LIBMAIX_ERR_NONE;
  }

  libmaix_err_t libmaix_cv_image_crop(struct libmaix_image *src, int x, int y, int w, int h, struct libmaix_image **dst)
  {
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    if (dst == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->width == 0 || src->height == 0 || src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    // int new_mem = libmaix_cv_image_load(src, dst);
    // -------------------------------
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat cv_src(src->height, src->width, CV_8UC3, src->data);
      cv::Mat dist;
      cv::Rect roi;
      roi.x = x;
      roi.y = y;
      roi.width = w;
      roi.height = h;
      cv_src(roi).copyTo(dist);
      memcpy((*dst)->data, dist.data, w * h * 3);
    }
    else
    {
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
      // -------------------------------
      // libmaix_cv_image_free(err, new_mem, dst);
      return LIBMAIX_ERR_NOT_IMPLEMENT;
    }
    // -------------------------------
    // libmaix_cv_image_free(err, new_mem, dst);
    return LIBMAIX_ERR_NONE;
  }

  libmaix_err_t libmaix_cv_image_rotate(libmaix_image_t *src, double rotate, int adjust, libmaix_image_t **dst)
  {
    libmaix_err_t err = LIBMAIX_ERR_NONE;
    if (dst == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    if (src->width == 0 || src->height == 0 || src->data == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    // -------------------------------
    if (src->mode == LIBMAIX_IMAGE_MODE_RGB888)
    {
      cv::Mat cv_src(src->height, src->width, CV_8UC3, src->data);
      cv::Mat cv_dist;
      if (adjust == 0)
      {
        cv::Point2f center((cv_src.cols - 1) / 2.0, (cv_src.rows - 1) / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, rotate, 1.0);
        cv::warpAffine(cv_src, cv_dist, rot, cv_src.size()); //the original size
        if (*dst == NULL)
        {
          *dst = libmaix_image_create(cv_dist.cols, cv_dist.rows, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        }
        if (*dst)
        {
          memcpy((*dst)->data, cv_dist.data, (*dst)->width * (*dst)->height * 3);
          return LIBMAIX_ERR_NONE;
        }
        else
        {
          return LIBMAIX_ERR_PARAM;
        }
      }
      else
      {
        double alpha = -rotate * CV_PI / 180.0; //convert angle to radian format
        cv::Point2f srcP[3];
        cv::Point2f dstP[3];
        srcP[0] = cv::Point2f(0, cv_src.rows);
        srcP[1] = cv::Point2f(cv_src.cols, 0);
        srcP[2] = cv::Point2f(cv_src.cols, cv_src.rows);
        //rotate the pixels
        for (int i = 0; i < 3; i++)
          dstP[i] = cv::Point2f(srcP[i].x * cos(alpha) - srcP[i].y * sin(alpha), srcP[i].y * cos(alpha) + srcP[i].x * sin(alpha));
        double minx, miny, maxx, maxy;
        minx = std::min(std::min(std::min(dstP[0].x, dstP[1].x), dstP[2].x), float(0.0));
        miny = std::min(std::min(std::min(dstP[0].y, dstP[1].y), dstP[2].y), float(0.0));
        maxx = std::max(std::max(std::max(dstP[0].x, dstP[1].x), dstP[2].x), float(0.0));
        maxy = std::max(std::max(std::max(dstP[0].y, dstP[1].y), dstP[2].y), float(0.0));
        int w = maxx - minx;
        int h = maxy - miny;
        //translation
        for (int i = 0; i < 3; i++)
        {
          if (minx < 0)
            dstP[i].x -= minx;
          if (miny < 0)
            dstP[i].y -= miny;
        }
        cv::Mat warpMat = cv::getAffineTransform(srcP, dstP);
        cv::warpAffine(cv_src, cv_dist, warpMat, cv::Size(w, h)); //extend size
        if (*dst != NULL)
          libmaix_image_destroy(dst);
        *dst = libmaix_image_create(cv_dist.cols, cv_dist.rows, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
        if (*dst)
        {
          memcpy((*dst)->data, cv_dist.data, (*dst)->width * (*dst)->height * 3);
          return LIBMAIX_ERR_NONE;
        }
        else
        {
          return LIBMAIX_ERR_PARAM;
        }
      } //end else
    }
    else
    {
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
      return LIBMAIX_ERR_NOT_IMPLEMENT;
    }
    // -------------------------------
    return LIBMAIX_ERR_NONE;
  }

  libmaix_image_color_t libmaix_cv_image_get_pixel(libmaix_image_t *src, int x, int y)
  {
    libmaix_image_color_t val;
    val.rgb888.a = 0;
    val.rgb888.b = 0;
    val.rgb888.g = 0;
    val.rgb888.r = 0;
    if (src->data == NULL)
    {
      return val;
    }
    switch (src->mode)
    {
    case LIBMAIX_IMAGE_MODE_RGB888:
    {
      cv::Mat input(src->height, src->width, CV_8UC3, const_cast<char *>((char *)src->data));
      val.rgb888.r = input.at<cv::Vec3b>(x, y)[0];
      val.rgb888.g = input.at<cv::Vec3b>(x, y)[1];
      val.rgb888.b = input.at<cv::Vec3b>(x, y)[2];
      return val;
    }
    break;
    case LIBMAIX_IMAGE_MODE_RGBA8888:
    {
      cv::Mat input(src->height, src->width, CV_8UC4, const_cast<char *>((char *)src->data));
      val.rgb888.r = input.at<cv::Vec4b>(x, y)[0];
      val.rgb888.g = input.at<cv::Vec4b>(x, y)[1];
      val.rgb888.b = input.at<cv::Vec4b>(x, y)[2];
      val.rgb888.a = input.at<cv::Vec4b>(x, y)[3];
      return val;
    }
    break;
    case LIBMAIX_IMAGE_MODE_GRAY:
    {
      cv::Mat input(src->height, src->width, CV_8UC1, const_cast<char *>((char *)src->data));
      val.rgb888.r = input.at<uchar>(x, y);
      return val;
    }
    break;
    default:
      break;
    }

    return val;
  }
  libmaix_err_t libmaix_cv_image_set_pixel(libmaix_image_t *src, int x, int y, libmaix_image_color_t color)
  {
    if (src == NULL)
    {
      return LIBMAIX_ERR_PARAM;
    }
    switch (src->mode)
    {
    case LIBMAIX_IMAGE_MODE_RGB888:
    {
      cv::Mat input(src->height, src->width, CV_8UC3, const_cast<char *>((char *)src->data));
      input.at<cv::Vec3b>(y, x)[0] = color.rgb888.r;
      input.at<cv::Vec3b>(y, x)[1] = color.rgb888.g;
      input.at<cv::Vec3b>(y, x)[2] = color.rgb888.b;
    }
    break;
    case LIBMAIX_IMAGE_MODE_RGBA8888:
    {
      cv::Mat input(src->height, src->width, CV_8UC4, const_cast<char *>((char *)src->data));
      input.at<cv::Vec4b>(y, x)[0] = color.rgb888.r;
      input.at<cv::Vec4b>(y, x)[1] = color.rgb888.g;
      input.at<cv::Vec4b>(y, x)[2] = color.rgb888.b;
      input.at<cv::Vec4b>(y, x)[3] = color.rgb888.a;
    }
    break;
    case LIBMAIX_IMAGE_MODE_GRAY:
    {
      cv::Mat input(src->height, src->width, CV_8UC1, const_cast<char *>((char *)src->data));
      input.at<uchar>(y, x) = color.rgb888.r;
    }
    break;
    default:
      LIBMAIX_IMAGE_ERROR(LIBMAIX_ERR_NOT_IMPLEMENT);
      return LIBMAIX_ERR_NOT_IMPLEMENT;
      break;
    }
    return LIBMAIX_ERR_NONE;
  }
}
