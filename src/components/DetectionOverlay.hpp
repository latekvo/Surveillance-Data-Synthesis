#pragma once

#include <string>
#include <vector>

#include "../types.h"
#include "BaseComponent.hpp"

namespace AS {

class DetectionOverlay : AS::BaseComponent {
 private:
  std::vector<Detection>* detectionsPtr;
  std::vector<std::string>* classesPtr;
  float* scalePtr;

 public:
  DetectionOverlay(std::vector<Detection>* detections,
                   std::vector<std::string>* classes, float* scale);

  void draw() override;
};

}  // namespace AS
