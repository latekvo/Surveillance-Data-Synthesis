#include "ObservedArea.hpp"

#include "../types.h"
#include "../types/point.hpp"

namespace AS {

ObservedArea::ObservedArea(CoordMap* coordMap, float* scale)
    : coordMapPtr(coordMap), scalePtr(scale) {}

void ObservedArea::draw() {
  const float scale = *this->scalePtr;
  const AS::Point<float>* mapPoints = this->coordMapPtr->cameraTrig.points;
  AS::Point<float> a = mapPoints[0] * scale, b = mapPoints[1] * scale,
                   c = mapPoints[2] * scale;

  const Vector2 vertexes[4] = {a.toRaylib(), b.toRaylib(), c.toRaylib(),
                               a.toRaylib()};
  DrawLineStrip(vertexes, 4, GREEN);

  DrawText("A", a.x, a.y, 16, PINK);
  DrawText("B", b.x, b.y, 16, PINK);
  DrawText("C", c.x, c.y, 16, PINK);
}

}  // namespace AS
