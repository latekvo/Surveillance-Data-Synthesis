#pragma once

#include "BaseComponent.hpp"

namespace AS {

class PixelPicker : AS::BaseComponent {
 private:
  float* scalePtr;

 public:
  PixelPicker(float* scale);

  void draw() override;
};

}  // namespace AS
