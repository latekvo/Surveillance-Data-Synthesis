#pragma once

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>

Rectangle scaleRect(Rectangle rect, float scale);
cv::Point scalePoint(cv::Point point, float scale);
std::vector<std::string> splitString(const std::string& txt, char ch);

template <typename T>
constexpr bool vectorContains(std::vector<T> vec, T element) {
  return std::find(vec.begin(), vec.end(), element) != vec.end();
}
