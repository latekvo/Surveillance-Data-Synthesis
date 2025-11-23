#pragma once

#include <onnxruntime_cxx_api.h>

#include <opencv2/opencv.hpp>

#include "types.h"

cv::Mat toLetterBox(cv::Mat frame, uint64_t size);
Ort::Value toYoloInputTensor(cv::Mat frame);
std::vector<DetectionArea> toDetectionAreas(cv::Mat& image, float areaSize);
