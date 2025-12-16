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
  // FIXME: The coords are currently used as px values, obv this is wrong

  CoordMap& map = *this->coordMapPtr;
  Triangle<float>& cTrig = map.cameraTrig;

  Triangle<float> oTrig{
      remapPointToReal(cTrig.a, map),
      remapPointToReal(cTrig.b, map),
      remapPointToReal(cTrig.c, map),
  };

  DrawText("A", oTrig.a.x, oTrig.a.y, 16, PINK);
  DrawText("B", oTrig.b.x, oTrig.b.y, 16, PINK);
  DrawText("C", oTrig.c.x, oTrig.c.y, 16, PINK);

  const Vector2 baVertexes[4] = {oTrig.a.toRaylib(), oTrig.b.toRaylib(),
                                 oTrig.c.toRaylib(), oTrig.a.toRaylib()};

  DrawLineStrip(baVertexes, 4, WHITE);

  for (const Detection& detection : *this->detectionsPtr) {
    Rectangle rawRect = detection.rect;

    AS::Point feet = AS::Point{rawRect.x + rawRect.width / 2 - 1,
                               rawRect.y + rawRect.height};

    AS::Point bFeet = remapPointToReal(feet, map);

    DrawRectangleLinesEx({bFeet.x, bFeet.y, 4, 4}, 3.f, PINK);
  }
}

}  // namespace AS
