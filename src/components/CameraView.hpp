#pragma once

#include <raylib.h>

#include "BaseComponent.hpp"

namespace AS {

class CameraView : AS::BaseComponent {
 private:
  Rectangle* boundsPtr;
  float* scalePtr;

 public:
  CameraView(Rectangle* bounds, float* scale);

  void draw() override;
};

}  // namespace AS
