#include "postprocess.h"

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <vector>

#include "types.h"
#include "utils.h"

std::vector<Detection> mergeDetectionAreas(std::vector<DetectionArea>& areas) {
  std::vector<Detection> merged;

  for (const DetectionArea& area : areas) {
    for (Detection detection : area.detections) {
      detection.rect.x += area.offset.x;
      detection.rect.y += area.offset.y;
      merged.push_back(detection);
    }
  }

  return merged;
}

std::vector<Detection> toFilteredDetections(
    std::vector<Detection>& detections, std::vector<uint>& allowedClassIDs) {
  std::vector<Detection> results;

  // This is very inefficient, but both vecs are too small for this to matter
  for (const Detection detection : detections) {
    if (vectorContains(allowedClassIDs, detection.classIdx)) {
      results.push_back(detection);
    }
  }

  return results;
}
