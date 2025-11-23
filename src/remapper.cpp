#include "remapper.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "consts.h"
#include "csv.h"
#include "types.h"

// TODO: Rewrite this entire thing into OOP registry (efficient lookup)

Triangle<float> pointsToTrig(std::vector<Point<float>>& points) {
  if (points.size() != 3) {
    throw std::runtime_error(
        "Observed areas configuration is invalid. Expected triangle.");
  }

  return {{{points[0].x, points[0].y},
           {points[1].x, points[1].y},
           {points[2].x, points[2].y}}};
}

void applyPointsToMap(CoordMap* coordMap,
                      std::vector<Point<float>>& cameraPoints,
                      std::vector<Point<float>>& realPoints) {
  coordMap->cameraTrig = pointsToTrig(cameraPoints);
  coordMap->realTrig = pointsToTrig(realPoints);
}

std::vector<CoordMap> loadCoordMaps() {
  std::vector<std::vector<std::string>> rawData = loadCsv(OBSERVED_AREAS_FILE);
  std::vector<CoordMap> coordMaps;
  std::vector<Point<float>> cameraPointsBuf, realPointsBuf;
  CoordMap* currentMapPtr = nullptr;

  for (const std::vector<std::string>& row : rawData) {
    const std::string& type = row[0];
    if (type == "ref") {
      if (currentMapPtr) {
        applyPointsToMap(currentMapPtr, cameraPointsBuf, realPointsBuf);
      }

      coordMaps.push_back({row[1]});
      currentMapPtr = &coordMaps.back();
      currentMapPtr->cameraRef = row[1];
    } else if (type == "vertex") {
      Point<float> camera = Point(std::stof(row[1]), std::stof(row[2])),
                   real = Point(std::stof(row[3]), std::stof(row[4]));

      cameraPointsBuf.push_back(camera);
      realPointsBuf.push_back(real);
    }
  }

  if (currentMapPtr) {
    applyPointsToMap(currentMapPtr, cameraPointsBuf, realPointsBuf);
  }

  return coordMaps;
}
cv::Point mapPointToCoords(cv::Point point, std::string cameraRef) {
  return {};
}
