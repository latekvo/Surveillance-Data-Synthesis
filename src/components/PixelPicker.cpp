#include "PixelPicker.hpp"

#include <raylib.h>

#include "../utils.h"

namespace AS {

PixelPicker::PixelPicker(float* scale) : scalePtr(scale) {}

void PixelPicker::draw() {
  Vector2 mousePos = GetMousePosition();

  Rectangle rect{mousePos.x, mousePos.y, 2, 2};
  Rectangle camPos = scaleRect(rect, 1 / *this->scalePtr);

  char xStr[64], yStr[64];
  std::snprintf(xStr, 64, "x: %d", (int)camPos.x);
  std::snprintf(yStr, 64, "y: %d", (int)camPos.y);

  DrawText(xStr, rect.x + 6, rect.y, 6, WHITE);
  DrawText(yStr, rect.x + 6, rect.y + 12, 6, WHITE);
  DrawRectangleLinesEx(rect, 2.f, WHITE);
}

}  // namespace AS
