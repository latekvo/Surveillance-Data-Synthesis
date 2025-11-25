#pragma once

#include <raylib.h>

#include <opencv2/opencv.hpp>

struct Detection {
  uint classIdx;
  float confidence;
  Rectangle rect;
};

struct DetectionArea {
  cv::Mat frame;
  cv::Point offset;
  std::vector<Detection> detections;
};

