#include "CameraView.hpp"

#include "../coco_labels.h"
#include "../types.h"
#include "DetectionOverlay.hpp"
#include "ObservedArea.hpp"

namespace AS {
CameraView::CameraView(Rectangle* bounds, float* scale,
                       std::vector<Detection>* detections, CoordMap* coordMap)
    : boundsPtr(bounds),
      scalePtr(scale),
      detectionsPtr(detections),
      coordMapPtr(coordMap),
      observedArea(coordMapPtr, scalePtr),
      detectionOverlay(detectionsPtr, &classes, scalePtr) {
  classes = getCocoLabels();
}

void CameraView::draw() {
  detectionOverlay.draw();
  observedArea.draw();
}

}  // namespace AS
