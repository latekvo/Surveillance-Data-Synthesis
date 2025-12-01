#pragma once

#include <vector>

#include "../remapper.h"
#include "../types.h"
#include "BaseComponent.hpp"

namespace AS {

class MinimapOverlay : AS::BaseComponent {
 private:
  std::vector<Detection>* detectionsPtr;
  CoordMap* coordMapPtr;
  float* scalePtr;

 public:
  MinimapOverlay(std::vector<Detection>* detections, CoordMap* coordMap,
                 float* scale);

  void draw() override;
};

}  // namespace AS
