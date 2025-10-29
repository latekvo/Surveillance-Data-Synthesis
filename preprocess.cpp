#include "preprocess.h"

#include <onnxruntime_cxx_api.h>

#include <opencv2/opencv.hpp>

cv::Mat toLetterBox(cv::Mat frame, uint64_t size) {
  uint x = frame.cols, y = frame.rows;
  uint max = std::max(x, y);
  float scale = float(size) / float(max);
  uint rX = std::floor(x * scale), rY = std::floor(y * scale);
  cv::Mat adjusted;
  cv::resize(frame, adjusted, cv::Size(rX, rY));
  cv::Mat output = cv::Mat::zeros(size, size, CV_8UC3);
  adjusted.copyTo(output(cv::Rect(0, 0, adjusted.cols, adjusted.rows)));
  return output;
}

Ort::Value toYoloInputTensor(cv::Mat frame) {
  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255.,
                         cv::Size(frame.cols, frame.rows), true, false, CV_32F);

  std::vector<int64_t> inputShape = {1, 3, frame.rows, frame.cols};
  int64_t inputSize =
      inputShape[0] * inputShape[1] * inputShape[2] * inputShape[3];

  Ort::MemoryInfo memoryInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  Ort::Value tensor = Ort::Value::CreateTensor<float>(
      memoryInfo, (float*)(blob.data), inputSize, inputShape.data(),
      inputShape.size());

  return tensor;
}
