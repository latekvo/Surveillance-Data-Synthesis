#pragma once

#include <vector>

#include "types.h"

struct CoordMap {
  std::string cameraRef;
  Triangle<float> cameraTrig;
  Triangle<float> realTrig;
};

std::vector<CoordMap> loadCoordMaps();

template <typename T>
Point<T> toBarycentric(Point<T> point, const Triangle<T>& trig) {
  // "Affinite transformation of the point"
  // We are aligning the triangle to cartesian 0 point, x and y axis,
  // and moving the point along the way
  Point<T> A = trig.points[0], B = trig.points[1], C = trig.points[2];
  Point<T> vecB = B - A;
  Point<T> vecC = C - A;

  // TODO: I'm sure these skews & other ops can be optimized

  // vecB is new Y axis, below is a skew to align vecB to Y
  T xRatioP = point.y / vecB.y;
  T xRatioC = vecC.y / vecB.y;
  vecC.x += vecC.x * xRatioC;  // 99% sure this is unnecessary
  point.x += point.x * xRatioP;

  // vecC is new X axis, below is a skew to align vecC to X
  T yRatioP = point.x / vecC.x;
  T yRatioB = vecB.x / vecC.x;
  vecB.y += vecB.y * yRatioB;  // 99% sure this is unnecessary
  point.y += point.y * yRatioP;

  // Scale such that both vecB & vecC are unitary
  point.x /= vecC.x;
  point.y /= vecB.y;

  return point;
}

template <typename T>
Point<T> fromBarycentric(Point<T> point, const Triangle<T>& trig) {
  // Exact reverse of toBarycentric
  return point;
}

template <typename T>
Point<T> remapPoint(Point<T> point, const CoordMap& coordMap) {
  point = toBarycentric(point, coordMap.cameraTrig);
  point = fromBarycentric(point, coordMap.realTrig);
  return point;
}

