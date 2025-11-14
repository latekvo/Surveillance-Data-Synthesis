#include "coco_labels.h"

#include <fstream>
#include <string>
#include <vector>

#include "consts.h"

std::vector<std::string> getCocoLabels() {
  std::vector<std::string> out;
  std::string line;
  std::ifstream file(CLASSES_FILE);

  if (file.is_open()) {
    while (getline(file, line)) {
      if (line[0] != '#') {
        out.push_back(line);
      }
    }

    file.close();
  }

  return out;
}
