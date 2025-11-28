#include "DetectionOverlay.hpp"

#include <string>
#include <vector>

#include "../types.h"
#include "../types/point.hpp"
#include "../utils.h"

namespace AS {

DetectionOverlay::DetectionOverlay(std::vector<Detection>* detections,
                                   std::vector<std::string>* classes,
                                   float* scale)
    : detectionsPtr(detections), classesPtr(classes), scalePtr(scale) {}

void DetectionOverlay::draw() {
  for (const Detection& detection : *this->detectionsPtr) {
    Rectangle rawRect = detection.rect;
    Rectangle rect = scaleRect(rawRect, *this->scalePtr);

    const auto classname = this->classesPtr->at(detection.classIdx).c_str();
    DrawText(classname, rect.x, rect.y - 10, 6, WHITE);
    DrawRectangleLinesEx(rect, 2.f, WHITE);

    AS::Point<float> feet = AS::Point{rawRect.x + rawRect.width / 2 - 1,
                                      rawRect.y + rawRect.height};

    AS::Point feetScaled = feet * *(this->scalePtr);
    DrawRectangleLinesEx({feetScaled.x, feetScaled.y, 4, 4}, 3.f, PINK);
  }
}

}  // namespace AS
