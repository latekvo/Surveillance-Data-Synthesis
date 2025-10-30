#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>

struct Detection {
  uint classIdx;
  float confidence;
  Rectangle rect;
};

std::vector<Detection> runDetection(Ort::Session& session, cv::Mat frame,
                                    std::vector<std::string> classList);
