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
AS::Point<T> toBarycentric(AS::Point<T> p, const Triangle<T>& trig) {
  AS::Point<T> a = trig.points[0], b = trig.points[1], c = trig.points[2];
  T v0x = b.x - a.x, v0y = b.y - a.y;
  T v1x = c.x - a.x, v1y = c.y - a.y;
  T v2x = p.x - a.x, v2y = p.y - a.y;
  T d00 = v0x * v0x + v0y * v0y;
  T d01 = v0x * v1x + v0y * v1y;
  T d11 = v1x * v1x + v1y * v1y;
  T d20 = v2x * v0x + v2y * v0y;
  T d21 = v2x * v1x + v2y * v1y;
  T denom = d00 * d11 - d01 * d01;
  p.x = (d11 * d20 - d01 * d21) / denom;
  p.y = (d00 * d21 - d01 * d20) / denom;
  return p;
}

template <typename T>
AS::Point<T> fromBarycentric(AS::Point<T> point, const Triangle<T>& trig) {
  // Exact reverse of toBarycentric
  return point;
}

template <typename T>
AS::Point<T> remapPoint(AS::Point<T> point, const CoordMap& coordMap) {
  point = toBarycentric(point, coordMap.cameraTrig);
  point = fromBarycentric(point, coordMap.realTrig);
  return point;
}

