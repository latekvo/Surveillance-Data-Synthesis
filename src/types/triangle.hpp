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

/*==============*\
 * Triangle x T *
\*==============*/

// --- x ---

template <typename T>
Triangle<T> operator+(const Triangle<T>& lhs, const T rhs) {
  return Triangle<T>{lhs.a + rhs, lhs.b + rhs, lhs.c + rhs};
}

template <typename T>
Triangle<T> operator*(const Triangle<T>& lhs, const T rhs) {
  return Triangle<T>{lhs.a * rhs, lhs.b * rhs, lhs.c * rhs};
}

// --- x= ---

template <typename T>
Triangle<T>& operator+=(Triangle<T>& lhs, const T rhs) {
  lhs.a += rhs;
  lhs.b += rhs;
  lhs.c += rhs;
  return lhs;
}

template <typename T>
Triangle<T>& operator*=(Triangle<T>& lhs, const T rhs) {
  lhs.a *= rhs;
  lhs.b *= rhs;
  lhs.c *= rhs;
  return lhs;
}

template <typename T>
Triangle<T>& operator/=(Triangle<T>& lhs, const T rhs) {
  lhs.a /= rhs;
  lhs.b /= rhs;
  lhs.c /= rhs;
  return lhs;
}

/*==================*\
 * Triangle x Point *
\*==================*/

// --- x ---

template <typename T>
Triangle<T> operator+(const Triangle<T>& lhs, const Point<T>& rhs) {
  return Triangle<T>{lhs.a + rhs, lhs.b + rhs, lhs.c + rhs};
}

template <typename T>
Triangle<T> operator-(const Triangle<T>& lhs, const Point<T>& rhs) {
  return Triangle<T>{lhs.a - rhs, lhs.b - rhs, lhs.c - rhs};
}

// --- x= ---

template <typename T>
Triangle<T>& operator+=(Triangle<T>& lhs, const Point<T>& rhs) {
  lhs.a += rhs;
  lhs.b += rhs;
  lhs.c += rhs;
  return lhs;
}

template <typename T>
Triangle<T>& operator-=(Triangle<T>& lhs, const Point<T>& rhs) {
  lhs.a -= rhs;
  lhs.b -= rhs;
  lhs.c -= rhs;
  return lhs;
}

}  // namespace AS
