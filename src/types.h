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

template <typename T>
struct Point {
  T x, y;
};

template <typename T>
Point<T> operator+(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x + rhs, lhs.y + rhs};
}

template <typename T>
Point<T> operator*(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x * rhs, lhs.y * rhs};
}

template <typename T>
Point<T> operator+(const Point<T>& lhs, const Point<T>& rhs) {
  return Point<T>{lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T>
Point<T> operator-(const Point<T>& lhs, const Point<T>& rhs) {
  return Point<T>{lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T>
struct Triangle {
  Point<T> points[3];
};
