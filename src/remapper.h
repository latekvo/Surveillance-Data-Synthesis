#pragma once

#include <vector>

#include "types.h"

struct CoordMap {
  std::string cameraRef;
  Triangle<float> cameraTrig;
  Triangle<float> realTrig;
};

std::vector<CoordMap> loadCoordMaps();
