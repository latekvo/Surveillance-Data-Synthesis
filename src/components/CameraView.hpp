#pragma once

#include <raylib.h>

#include <string>
#include <vector>

#include "../types.h"
#include "BaseComponent.hpp"
#include "DetectionOverlay.hpp"
#include "ObservedArea.hpp"

namespace AS {

class CameraView : AS::BaseComponent {
 private:
  Rectangle* boundsPtr;
  float* scalePtr;
  std::vector<Detection>* detectionsPtr;
  std::vector<std::string> classes;
  CoordMap* coordMapPtr;
  DetectionOverlay detectionOverlay;
  ObservedArea observedArea;

 public:
  CameraView(Rectangle* bounds, float* scale,
             std::vector<Detection>* detections, CoordMap* coordMap);

  void draw() override;
};

}  // namespace AS
