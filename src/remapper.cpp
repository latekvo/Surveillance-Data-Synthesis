#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "consts.h"
#include "list_parser.h"
// 1. load coord maps
// 2. process coordinates

struct CoordMapTrig {
  uint x1, y1, x2, y2, x3, y3;  // camera coords
  uint p1, q1, p2, q2, p3, q3;  // real coords
};

struct CoordMap {
  std::string cameraRef;  // TODO: Make this the key, instead of a trivial label
  CoordMapTrig cameraTrig;  // FIXME: using just one for testing
  // std::vector<CoordMapTrig> cameraTrigs;
  CoordMapTrig realTrig;
  // std::vector<CoordMapTrig> realTrigs;
};

// TODO: Rewrite this entire thing into OOP registry

// I don't think we actually care about whether the point inside.
// The perspective transform should work regardless.
bool isWithinTrig(cv::Point point) { return true; }
bool isWithinMap(cv::Point point) { return true; }

std::vector<CoordMap> loadCoordMaps() {
  std::vector<std::string> rawData = parseListFile(OBSERVED_AREAS_FILE);
  std::string cameraRef = rawData[0];
  std::string v1Data = rawData[1], v2Data = rawData[2], v3Data = rawData[3];

  std::vector<cv::Point> cameraVertexes, realVertexes;

  // TODO: handle multiple entries

  auto coordMap = CoordMap();

  return {coordMap};
}
cv::Point mapPointToCoords(cv::Point point, std::string cameraRef) {
  return {};
}
