#pragma once

#include <vector>

#include "../types.h"
#include "BaseComponent.hpp"

namespace AS {

class ObservedArea : AS::BaseComponent {
 private:
  std::vector<Detection>* detectionsPtr;
  CoordMap* coordMapPtr;
  float* scalePtr;

 public:
  ObservedArea(CoordMap* coordMap, float* scale);

  void draw() override;
};

}  // namespace AS
