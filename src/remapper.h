#pragma once

#include <string>
#include <vector>

#include "types/triangle.hpp"

struct CoordMap {
  std::string cameraRef;
  AS::Triangle<float> cameraTrig;
  AS::Triangle<float> realTrig;
};

std::vector<CoordMap> loadCoordMaps();

template <typename T>
AS::Point<T> toBarycentric(AS::Point<T> p, const AS::Triangle<T>& t) {
  T v0x = t.b.x - t.a.x, v0y = t.b.y - t.a.y;
  T v1x = t.c.x - t.a.x, v1y = t.c.y - t.a.y;
  T v2x = p.x - t.a.x, v2y = p.y - t.a.y;
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
AS::Point<T> fromBarycentric(const AS::Point<T>& p,
                             const AS::Triangle<T>& trig) {
  T z = 1 - p.x - p.y;
  return AS::Point{trig.a.x * p.x + trig.b.x * p.y + trig.c.x * z,
                   trig.a.y * p.x + trig.b.y * p.y + trig.c.y * z};
}

template <typename T>
AS::Point<T> remapPoint(const AS::Point<T>& point, const CoordMap& coordMap) {
  AS::Point bPoint = fromBarycentric(point, coordMap.realTrig);
  return toBarycentric(bPoint, coordMap.cameraTrig);
}

