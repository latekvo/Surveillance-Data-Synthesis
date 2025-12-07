#include "utils.h"

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

Rectangle scaleRect(Rectangle& rect, float scale) {
  return Rectangle{
      rect.x * scale,
      rect.y * scale,
      rect.width * scale,
      rect.height * scale,
  };
}

std::vector<std::string> splitString(const std::string& txt, char ch) {
  size_t pos = txt.find(ch);
  size_t initialPos = 0;
  std::vector<std::string> strings;

  while (pos != std::string::npos) {
    strings.push_back(txt.substr(initialPos, pos - initialPos));
    initialPos = pos + 1;

    pos = txt.find(ch, initialPos);
  }

  strings.push_back(
      txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

  return strings;
}
