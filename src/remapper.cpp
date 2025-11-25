#include "remapper.h"

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

#include "consts.h"
#include "csv.h"
#include "types/point.hpp"
#include "types/triangle.hpp"

// TODO: Rewrite this entire thing into OOP registry (efficient lookup)

AS::Triangle<float> pointsToTrig(std::vector<AS::Point<float>>& points) {
  if (points.size() != 3) {
    throw std::runtime_error(
        std::format("Observed areas configuration is invalid. "
                    "Expected triangle. Got {}-angle.",
                    points.size()));
  }

  return AS::Triangle<float>{points[0], points[1], points[2]};
}

void applyPointsToMap(CoordMap* coordMap,
                      std::vector<AS::Point<float>>& cameraPoints,
                      std::vector<AS::Point<float>>& realPoints) {
  coordMap->cameraTrig = pointsToTrig(cameraPoints);
  coordMap->realTrig = pointsToTrig(realPoints);
}

std::vector<CoordMap> loadCoordMaps() {
  std::vector<std::vector<std::string>> rawData = loadCsv(OBSERVED_AREAS_FILE);
  std::vector<CoordMap> coordMaps;
  std::vector<AS::Point<float>> cameraPointsBuf, realPointsBuf;
  CoordMap* currentMapPtr = nullptr;

  for (const std::vector<std::string>& row : rawData) {
    const std::string& type = row[0];
    if (type == "ref") {
      if (currentMapPtr) {
        applyPointsToMap(currentMapPtr, cameraPointsBuf, realPointsBuf);
        cameraPointsBuf.clear();
        realPointsBuf.clear();
      }

      coordMaps.push_back({row[1]});
      currentMapPtr = &coordMaps.back();
      currentMapPtr->cameraRef = row[1];
    } else if (type == "vertex") {
      AS::Point<float> camera = AS::Point(std::stof(row[1]), std::stof(row[2])),
                       real = AS::Point(std::stof(row[3]), std::stof(row[4]));

      cameraPointsBuf.push_back(camera);
      realPointsBuf.push_back(real);
    }
  }

  if (currentMapPtr) {
    applyPointsToMap(currentMapPtr, cameraPointsBuf, realPointsBuf);
  }

  return coordMaps;
}
