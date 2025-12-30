#pragma once

#include <raylib.h>

#include <memory>
#include <string>
#include <vector>

#include "../types.h"
#include "BaseComponent.hpp"
#include "DetectionOverlay.hpp"
#include "ObservedArea.hpp"

namespace AS {

class CameraView : AS::BaseComponent {
 private:
  Rectangle* boundsPtr;
  float* scalePtr;
  std::vector<Detection>* detectionsPtr;
  std::vector<std::string> classes;
  CoordMap* coordMapPtr;
  DetectionOverlay detectionOverlay;
  ObservedArea observedArea;
  Image rayImage;
  cv::Mat* videoFramePtr;

  std::unique_ptr<Texture2D> texturePtr;

 public:
  CameraView(Rectangle* bounds, float* scale,
             std::vector<Detection>* detections, CoordMap* coordMap,
             cv::Mat* videoFrame);

  void draw() override;
  void postRender() override;
};

}  // namespace AS
