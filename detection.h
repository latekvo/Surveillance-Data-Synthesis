#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>

#include "types.h"

std::vector<Detection> runDetection(Ort::Session& session, cv::Mat frame,
                                    std::vector<std::string> classList);
