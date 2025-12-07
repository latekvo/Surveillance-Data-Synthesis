#pragma once

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>

#include "types.h"

std::vector<Detection> getDetectionsFromFrame(
    Ort::Session& session, cv::Mat& input, std::vector<std::string>& classes);
