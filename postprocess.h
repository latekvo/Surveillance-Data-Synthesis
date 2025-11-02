#pragma once

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "types.h"

std::vector<Detection> mergeDetectionAreas(std::vector<DetectionArea>& areas);
std::vector<Detection> toFilteredDetections(std::vector<Detection>& detections,
                                            std::vector<uint>& allowedClassIDs);
