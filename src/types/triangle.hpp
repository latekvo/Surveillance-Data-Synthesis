#pragma once

#include "point.hpp"

namespace AS {

template <typename T>
struct Triangle {
  union {
    AS::Point<T> points[3];
    struct {
      Point<T> a, b, c;
    };
  };
};

template <typename T>
Triangle<T> operator*(const Triangle<T>& lhs, const T& rhs) {
  return Triangle<T>{lhs.a * rhs, lhs.b * rhs, lhs.c * rhs};
}

}  // namespace AS
