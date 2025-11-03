#include "utils.h"

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

Rectangle scaleRect(Rectangle rect, float scale) {
  return Rectangle{
      rect.x * scale,
      rect.y * scale,
      rect.width * scale,
      rect.height * scale,
  };
}

cv::Point scalePoint(cv::Point point, float scale) {
  return cv::Point(point.x * scale, point.y * scale);
}
