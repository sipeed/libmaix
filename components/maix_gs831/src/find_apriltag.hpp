
#include "gs831_uvai.hpp"

extern "C"
{
  extern gs831_uv *gs831;

  // ==============================================================================================

#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "tagCircle21h7.h"
#include "tagCircle49h12.h"
#include "tagCustom48h12.h"
#include "tagStandard41h12.h"
#include "tagStandard52h13.h"

  static struct _find_apriltag_
  {
    apriltag_detector_t *td = NULL;
    apriltag_family_t *tf = NULL;
    //读取相机内参和畸变矩阵默认值
    float fx = static_cast<float>((6 / 5.76) * 240);
    float fy = static_cast<float>((6 / 3.24) * 240);
    float cx = static_cast<float>(240 / 2);
    float cy = static_cast<float>(180 / 2);
    std::string famname = "tagStandard52h13";
    float tag_size = 100; // mm apriltag size
    float quad_decimate = 2.0;
    float quad_sigma = 0.0;
    float decode_sharpening = 0.25;
    int refine_edges = 1;
    int bits_corrected = 0;
    std::vector<cv::Point3f> object_point;
  } find_apriltag_app;

  // ==============================================================================================

  int find_apriltag_app_load(_find_apriltag_ *self)
  {
    gs831_load_json_conf(gs831->config_file, gs831->config_json, json5pp::object({
        {"fx", self->fx},
        {"fx", self->fy},
        {"cx", self->cx},
        {"cy", self->cy},
        {"tag_size", self->tag_size},
        {"quad_decimate", self->quad_decimate},
        {"quad_sigma", self->quad_sigma},
        {"refine_edges", self->refine_edges},
        {"bits_corrected", self->bits_corrected},
        {"decode_sharpening", self->decode_sharpening},
        {"famname", self->famname},
    }));

    {
      {
        auto result = gs831->config_json["fx"];
        if (result.is_number()) self->fx = result.as_integer();
      }
      {
        auto result = gs831->config_json["fy"];
        if (result.is_number()) self->fy = result.as_integer();
      }
      {
        auto result = gs831->config_json["cx"];
        if (result.is_number()) self->cx = result.as_integer();
      }
      {
        auto result = gs831->config_json["cy"];
        if (result.is_number()) self->cy = result.as_integer();
      }
      {
        auto result = gs831->config_json["tag_size"];
        if (result.is_number()) self->tag_size = result.as_integer();
      }
      {
        auto result = gs831->config_json["quad_decimate"];
        if (result.is_number()) self->quad_decimate = result.as_integer();
      }
      {
        auto result = gs831->config_json["quad_sigma"];
        if (result.is_number()) self->quad_sigma = result.as_integer();
      }
      {
        auto result = gs831->config_json["refine_edges"];
        if (result.is_number()) self->refine_edges = result.as_integer();
      }
      {
        auto result = gs831->config_json["bits_corrected"];
        if (result.is_number()) self->bits_corrected = result.as_integer();
      }
      {
        auto result = gs831->config_json["decode_sharpening"];
        if (result.is_number()) self->decode_sharpening = result.as_integer();
      }
      {
        auto result = gs831->config_json["famname"];
        if (result.is_string()) self->famname = result.as_string();
      }
      gs831_save_json_conf(gs831->config_file, gs831->config_json);
    }

    self->object_point.push_back(cv::Point3f(-self->tag_size / 2, -self->tag_size / 2, 0));
    self->object_point.push_back(cv::Point3f(self->tag_size / 2, -self->tag_size / 2, 0));
    self->object_point.push_back(cv::Point3f(self->tag_size / 2, self->tag_size / 2, 0));
    self->object_point.push_back(cv::Point3f(-self->tag_size / 2, self->tag_size / 2, 0));

    {
      // Initialize tag detector with options
      if (!strcmp(self->famname.c_str(), "tag36h11"))
      {
        self->tf = tag36h11_create();
      }
      else if (!strcmp(self->famname.c_str(), "tag25h9"))
      {
        self->tf = tag25h9_create();
      }
      else if (!strcmp(self->famname.c_str(), "tag16h5"))
      {
        self->tf = tag16h5_create();
      }
      else if (!strcmp(self->famname.c_str(), "tagCircle21h7"))
      {
        self->tf = tagCircle21h7_create();
      }
      else if (!strcmp(self->famname.c_str(), "tagCircle49h12"))
      {
        self->tf = tagCircle49h12_create();
      }
      else if (!strcmp(self->famname.c_str(), "tagStandard41h12"))
      {
        self->tf = tagStandard41h12_create();
      }
      else if (!strcmp(self->famname.c_str(), "tagStandard52h13"))
      {
        self->tf = tagStandard52h13_create();
      }
      else if (!strcmp(self->famname.c_str(), "tagCustom48h12"))
      {
        self->tf = tagCustom48h12_create();
      }
      else
      {
        printf("Unrecognized tag family name. Use e.g. \"tag36h11\".\n");
        exit(-1);
      }
      // apriltag_detector_add_family(td, tf);
      self->td = apriltag_detector_create();
      apriltag_detector_add_family_bits(self->td, self->tf, self->bits_corrected);
      self->td->quad_decimate = self->quad_decimate; // "Decimate input image by this factor"
      self->td->quad_sigma = self->quad_sigma;    // Apply low-pass blur to input
      self->td->refine_edges = self->refine_edges; // Spend more time trying to align edges of tags
      self->td->decode_sharpening = self->decode_sharpening; // Default = 0.25
      self->td->nthreads = 2;
      self->td->debug = 0;
    }

    LIBMAIX_INFO_PRINTF("find_apriltag_app_load");
    if (self->td && self->tf)
    {
      return 0;
    }
    return -1;
  }

  int find_apriltag_app_exit(_find_apriltag_ *self)
  {
    if (self->td && self->tf)
    {
      apriltag_detector_destroy(self->td), self->td = NULL;
      if (!strcmp(self->famname.c_str(), "tag36h11"))
      {
        tag36h11_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tag25h9"))
      {
        tag25h9_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tag16h5"))
      {
        tag16h5_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tagCircle21h7"))
      {
        tagCircle21h7_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tagCircle49h12"))
      {
        tagCircle49h12_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tagStandard41h12"))
      {
        tagStandard41h12_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tagStandard52h13"))
      {
        tagStandard52h13_destroy(self->tf);
      }
      else if (!strcmp(self->famname.c_str(), "tagCustom48h12"))
      {
        tagCustom48h12_destroy(self->tf);
      }
      self->tf = NULL;
    }

    LIBMAIX_INFO_PRINTF("find_apriltag_app_exit");
    return 0;
  }

  static inline uint32_t gs831_get_ms()
  {
    static struct timespec tmp;
    clock_gettime(CLOCK_MONOTONIC, &tmp);
    return (tmp.tv_sec * 1000) + (uint32_t)tmp.tv_nsec / 1000000;
  }

  int find_apriltag_app_loop(_find_apriltag_ *self, uint8_t *data, uint32_t width, uint32_t height)
  {
    if (self->td && self->tf)
    {
      struct apriltag_data
      {
        uint8_t head;
        uint8_t len;
        uint8_t retain_0;
        uint8_t retain_1;
        uint32_t tm;
        uint32_t id;
        float decision_margin;
        float center[2];
        float points[4][2];
        float rotation[3][3];
        uint8_t retain_2;
        uint8_t retain_3;
        uint8_t sum;
        uint8_t end;
      } upload_data = {0x55, sizeof(struct apriltag_data), 0, 0, gs831_get_ms(), 0, 0, {0, 0}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0, 0, 0, 0x0A};

      cv::Mat gray(height, width, CV_8UC1, data);

      // Make an image_u8_t header for the Mat data
      image_u8_t im = {
          .width = gray.cols,
          .height = gray.rows,
          .stride = gray.cols,
          .buf = (uint8_t *)(gray.data)};

      zarray_t *detections = apriltag_detector_detect(self->td, &im);

      // Draw detection outlines
      for (int i = 0; i < zarray_size(detections); i++)
      {

        apriltag_detection_t *det;
        zarray_get(detections, i, &det);

        cv::line(gray, cv::Point(det->p[0][0], det->p[0][1]),
                 cv::Point(det->p[1][0], det->p[1][1]),
                 cv::Scalar(0xff, 0, 0), 2);
        cv::line(gray, cv::Point(det->p[1][0], det->p[1][1]),
                 cv::Point(det->p[2][0], det->p[2][1]),
                 cv::Scalar(0xff, 0, 0), 2);
        cv::line(gray, cv::Point(det->p[2][0], det->p[2][1]),
                 cv::Point(det->p[3][0], det->p[3][1]),
                 cv::Scalar(0xff, 0, 0), 2);
        cv::line(gray, cv::Point(det->p[3][0], det->p[3][1]),
                 cv::Point(det->p[0][0], det->p[0][1]),
                 cv::Scalar(0xff, 0, 0), 2);

        upload_data.id = det->id;
        upload_data.decision_margin = det->decision_margin;
        upload_data.center[0] = det->c[0];
        upload_data.center[1] = det->c[1];
        upload_data.points[0][0] = det->p[0][0];
        upload_data.points[0][1] = det->p[0][1];
        upload_data.points[1][0] = det->p[1][0];
        upload_data.points[1][1] = det->p[1][1];
        upload_data.points[2][0] = det->p[2][0];
        upload_data.points[2][1] = det->p[2][1];
        upload_data.points[3][0] = det->p[3][0];
        upload_data.points[3][1] = det->p[3][1];

        cv::Matx33d camera_matrix(self->fx, 0, self->cx,
                                  0, self->fy, self->cy,
                                  0, 0, 1);

        cv::Vec4f distortion_coeffs(0, 0, 0, 0); // distortion coefficients

        cv::Mat rvec = cv::Mat::zeros(3, 3, CV_64FC1);
        cv::Mat tvec = cv::Mat::zeros(3, 1, CV_64FC1);

        std::vector<cv::Point2f> image_point;
        image_point.push_back(cv::Point2d(det->p[0][0], det->p[0][1]));
        image_point.push_back(cv::Point2d(det->p[1][0], det->p[1][1]));
        image_point.push_back(cv::Point2d(det->p[2][0], det->p[2][1]));
        image_point.push_back(cv::Point2d(det->p[3][0], det->p[3][1]));

        cv::solvePnP(self->object_point, image_point, camera_matrix, distortion_coeffs, rvec, tvec);

        cv::Matx33d R;

        cv::Rodrigues(rvec, R);

        // auto text = string_format("[%0.02f, %0.02f, %0.02f, %0.02f, %0.02f, %0.02f, %0.02f, %0.02f, %0.02f]", R(0,0), R(0,1), R(0,2), R(1,0), R(1,1), R(1,2), R(2,0), R(2,1), R(2,2));
        // printf("text %s\r\n", text.c_str());

        upload_data.rotation[0][0] = R(0,0);
        upload_data.rotation[0][1] = R(0,1);
        upload_data.rotation[0][2] = R(0,2);
        upload_data.rotation[1][0] = R(1,0);
        upload_data.rotation[1][1] = R(1,1);
        upload_data.rotation[1][2] = R(1,2);
        upload_data.rotation[2][0] = R(2,0);
        upload_data.rotation[2][1] = R(2,1);
        upload_data.rotation[2][2] = R(2,2);

        // Eigen::Matrix3d wRo;
        // wRo << R(0,0), R(0,1), R(0,2), R(1,0), R(1,1), R(1,2), R(2,0), R(2,1), R(2,2);

        // Eigen::Matrix4d T; // homogeneous transformation matrix
        // T.topLeftCorner(3, 3) = wRo;
        // T.col(3).head(3) <<
        //     tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2);
        // T.row(3) << 0,0,0,1;
        // return T;

        uint8_t *ptr = (uint8_t *)&upload_data;
        // uint8_t sum = 0;
        for (int i = 0; i < upload_data.len - 2; i++)
          upload_data.sum += ptr[i];
        // upload_data.sum = sum;
        write(gs831->dev_ttyS, ptr, upload_data.len);

        break;

        // cv::Matx<double, 3, 3> R_inv = R.t();

        // auto text = string_format("[(%d,%.0f),(%d,%d)]\r\n", det->id, det->decision_margin, (int)det->c[0], (int)det->c[1]);

        // write(gs831->dev_ttyS, text.c_str(), text.length());

        // printf("detection %d: id=%d, hamming=%d, decision_margin=%f, "
        //        "p[0]=(%f,%f), p[1]=(%f,%f), p[2]=(%f,%f), p[3]=(%f,%f)\n",
        //        i, det->id, det->hamming, det->decision_margin,
        //        det->p[0][0], det->p[0][1], det->p[1][0], det->p[1][1],
        //        det->p[2][0], det->p[2][1], det->p[3][0], det->p[3][1]);

        // int fontface = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
        // double fontscale = 1.0;
        // int baseline;
        // cv::Size textsize = cv::getTextSize(text, fontface, fontscale, 2,
        //                                 &baseline);
        // cv::putText(gray, text, cv::Point(det->c[0]-textsize.width/2,
        //                             det->c[1]+textsize.height/2),
        //         fontface, fontscale, cv::Scalar(0xff, 0x99, 0), 2);
      }

      apriltag_detections_destroy(detections);
    }

    return 0;
  }
}
