#include "MinimapOverlay.hpp"

#include <vector>

#include "../remapper.h"
#include "../types.h"
#include "../types/point.hpp"

namespace AS {

MinimapOverlay::MinimapOverlay(std::vector<Detection>* detections,
                               CoordMap* coordMap, float* scale)
    : detectionsPtr(detections), coordMapPtr(coordMap), scalePtr(scale) {}

void MinimapOverlay::draw() {
  float baryScale = 100;

  // TODO: Also draw camera bounds (0:0/1920:1080) on the minimap

  Triangle<float> bTrig{toBarycentric(this->coordMapPtr->cameraTrig.a,
                                      this->coordMapPtr->cameraTrig),
                        toBarycentric(this->coordMapPtr->cameraTrig.b,
                                      this->coordMapPtr->cameraTrig),
                        toBarycentric(this->coordMapPtr->cameraTrig.c,
                                      this->coordMapPtr->cameraTrig)};

  bTrig = bTrig * baryScale + baryScale;

  DrawText("A", bTrig.a.x, bTrig.a.y, 16, PINK);
  DrawText("B", bTrig.b.x, bTrig.b.y, 16, PINK);
  DrawText("C", bTrig.c.x, bTrig.c.y, 16, PINK);

  const Vector2 baVertexes[4] = {bTrig.a.toRaylib(), bTrig.b.toRaylib(),
                                 bTrig.c.toRaylib(), bTrig.a.toRaylib()};

  DrawLineStrip(baVertexes, 4, WHITE);

  for (const Detection& detection : *this->detectionsPtr) {
    Rectangle rawRect = detection.rect;

    AS::Point feet = AS::Point{rawRect.x + rawRect.width / 2 - 1,
                               rawRect.y + rawRect.height};

    AS::Point bFeet = toBarycentric(feet, this->coordMapPtr->cameraTrig);
    bFeet = bFeet * baryScale + baryScale;

    DrawRectangleLinesEx({bFeet.x, bFeet.y, 4, 4}, 3.f, PINK);
  }
}

}  // namespace AS
