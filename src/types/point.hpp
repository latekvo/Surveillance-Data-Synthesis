#pragma once

namespace AS {

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

}  // namespace AS
