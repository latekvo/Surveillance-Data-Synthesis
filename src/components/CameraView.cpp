#include "CameraView.hpp"

#include "../coco_labels.h"
#include "../types.h"
#include "DetectionOverlay.hpp"
#include "ObservedArea.hpp"

namespace AS {
CameraView::CameraView(Rectangle* bounds, float* scale,
                       std::vector<Detection>* detections, CoordMap* coordMap,
                       cv::Mat* videoFrame)
    : boundsPtr(bounds),
      scalePtr(scale),
      detectionsPtr(detections),
      coordMapPtr(coordMap),
      observedArea(coordMapPtr, scalePtr),
      detectionOverlay(detectionsPtr, &classes, scalePtr),
      videoFramePtr(videoFrame) {
  classes = getCocoLabels();
  rayImage.mipmaps = 1;
  rayImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
}

void CameraView::draw() {
  // Remap OpenCV to raylib image, no copy, using shared memory
  rayImage.data = videoFramePtr->data;
  rayImage.width = videoFramePtr->cols;
  rayImage.height = videoFramePtr->rows;

  Texture2D texture = LoadTextureFromImage(rayImage);
  texturePtr = std::make_unique<Texture2D>(texture);

  DrawTexture(texture, 0, 0, WHITE);
  detectionOverlay.draw();
  observedArea.draw();
}

void CameraView::postRender() { UnloadTexture(*texturePtr.get()); }

}  // namespace AS
