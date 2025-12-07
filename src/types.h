#pragma once

#include <raylib.h>

#include <opencv2/opencv.hpp>

#include "types/triangle.hpp"

typedef unsigned int uint;

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

struct CoordMap {
  std::string cameraRef;
  AS::Triangle<float> cameraTrig;
  AS::Triangle<float> realTrig;
};

struct Stream {
  std::string name;
  std::string url;
};
