#pragma once

#include <vector>

#include "types.h"

struct CoordMap {
  std::string cameraRef;
  Triangle<double> cameraTrig;
  Triangle<double> realTrig;
};

std::vector<CoordMap> loadCoordMaps();
