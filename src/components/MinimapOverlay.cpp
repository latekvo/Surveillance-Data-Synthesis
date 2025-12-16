#include "MinimapOverlay.hpp"

#include <vector>

#include "../remapper.h"
#include "../types.h"
#include "../types/point.hpp"

namespace AS {

// This is a temporary solution.
// We'll want to adjust recangles into bounds in the future.
// FIXME: We want to apply this to minimap detections as well
// TODO: Move to centralized detections registry
void resizeTriangle(AS::Triangle<float>& t, float squareSize) {
  Point<float> shift{t.a.x, t.a.y};

  if (t.b.x < shift.x) shift.x = t.b.x;
  if (t.c.x < shift.x) shift.x = t.c.x;
  if (t.b.y < shift.y) shift.y = t.b.y;
  if (t.c.y < shift.y) shift.y = t.c.y;

  t -= shift;

  float scale = t.a.x;

  if (t.b.x > scale) scale = t.b.x;
  if (t.c.x > scale) scale = t.c.x;
  if (t.a.y > scale) scale = t.a.y;
  if (t.b.y > scale) scale = t.b.y;
  if (t.c.y > scale) scale = t.c.y;

  t /= scale;
  t *= squareSize;
}

MinimapOverlay::MinimapOverlay(std::vector<Detection>* detections,
                               CoordMap* coordMap, float* scale)
    : detectionsPtr(detections), coordMapPtr(coordMap), scalePtr(scale) {}

void MinimapOverlay::draw() {
  float baryScale = 100;

  // TODO: Also draw camera bounds (0:0/1920:1080) on the minimap
  // FIXME: The coords are currently used as px values, obv this is wrong

  CoordMap& map = *this->coordMapPtr;
  Triangle<float>& cTrig = map.cameraTrig;

  Triangle<float> rTrig{
      remapPointToReal(cTrig.a, map),
      remapPointToReal(cTrig.b, map),
      remapPointToReal(cTrig.c, map),
  };

  resizeTriangle(rTrig, 100.f);
  rTrig += 50.f;

  DrawText("A", rTrig.a.x, rTrig.a.y, 16, PINK);
  DrawText("B", rTrig.b.x, rTrig.b.y, 16, PINK);
  DrawText("C", rTrig.c.x, rTrig.c.y, 16, PINK);

  const Vector2 baVertexes[4] = {rTrig.a.toRaylib(), rTrig.b.toRaylib(),
                                 rTrig.c.toRaylib(), rTrig.a.toRaylib()};

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
