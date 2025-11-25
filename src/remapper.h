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
Point<T> toBarycentric(Point<T> p, const Triangle<T>& trig) {
  Point<T> a = trig.points[0], b = trig.points[1], c = trig.points[2];
  T denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
  p.x = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / denom;
  p.y = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / denom;
  return p;
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

