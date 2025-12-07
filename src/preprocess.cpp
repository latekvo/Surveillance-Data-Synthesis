#include "preprocess.h"

#include <onnxruntime_cxx_api.h>

#include <opencv2/opencv.hpp>

cv::Mat toLetterBox(cv::Mat& frame, uint64_t size) {
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

Ort::Value toYoloInputTensor(cv::Mat& frame) {
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

std::vector<DetectionArea> toDetectionAreas(cv::Mat& image, float areaSize) {
  std::vector<DetectionArea> areas;

  // TODO: Introduce overlap (10-20px?)

  const uint xCount = uint(std::ceil(float(image.cols) / areaSize));
  const uint yCount = uint(std::ceil(float(image.rows) / areaSize));

  for (uint y = 0; y < yCount; y++) {
    for (uint x = 0; x < xCount; x++) {
      DetectionArea area;
      const uint xOffset = uint(x * areaSize);
      const uint yOffset = uint(y * areaSize);
      uint width = areaSize;
      uint height = areaSize;
      uint widthRemainder = image.cols % uint(areaSize);
      uint heightRemainder = image.rows % uint(areaSize);

      if (x == xCount - 1 && widthRemainder != 0) {
        width = widthRemainder;
      }

      if (y == yCount - 1 && heightRemainder != 0) {
        height = heightRemainder;
      }

      cv::Mat cropped = image(cv::Rect(xOffset, yOffset, width, height));
      cv::Mat letterBox = toLetterBox(cropped, areaSize);

      area.offset = cv::Point(xOffset, yOffset);
      area.frame = letterBox;
      areas.push_back(area);
    }
  }

  return areas;
}
