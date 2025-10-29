#include <onnxruntime_cxx_api.h>

#include <opencv2/opencv.hpp>

cv::Mat toLetterBox(cv::Mat frame, uint64_t size);
Ort::Value toYoloInputTensor(cv::Mat frame);
