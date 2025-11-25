#pragma once

#include <raylib.h>

#include <opencv2/core.hpp>

namespace AS {

template <typename T>
struct Point {
  T x, y;

  cv::Point toCV();
  Vector2 toRaylib();
};

template <typename T>
cv::Point Point<T>::toCV() {
  return cv::Point(this->x, this->y);
}

template <typename T>
Vector2 Point<T>::toRaylib() {
  return Vector2(this->x, this->y);
}

template <typename T>
Point<T> operator+(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x + rhs, lhs.y + rhs};
}

template <typename T>
Point<T> operator-(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x - rhs, lhs.y - rhs};
}

template <typename T>
Point<T> operator*(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x * rhs, lhs.y * rhs};
}

template <typename T>
Point<T> operator/(const Point<T>& lhs, const T& rhs) {
  return Point<T>{lhs.x / rhs, lhs.y / rhs};
}

template <typename T>
Point<T> operator+(const Point<T>& lhs, const Point<T>& rhs) {
  return Point<T>{lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T>
Point<T> operator-(const Point<T>& lhs, const Point<T>& rhs) {
  return Point<T>{lhs.x - rhs.x, lhs.y - rhs.y};
}

}  // namespace AS
