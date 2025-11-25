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
  Point<T> a = trig.points[0], b = trig.points[1], c = trig.points[2];

  T xRatio = point.y / (c.y - b.y);
  T xVec = a.x - c.x;
  point.x += xVec * xRatio;

  T yRatio = point.x / (b.x - c.x);
  T yVec = a.y - b.y;
  point.y += yVec * yRatio;

  // Now we have a right triangle, next adjusting it's size

  point.x /= b.x - a.x;
  point.y /= c.y - a.y;

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

