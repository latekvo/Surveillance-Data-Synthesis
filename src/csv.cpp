#include "csv.h"

#include <fstream>
#include <string>
#include <vector>

#include "utils.h"

std::vector<std::vector<std::string>> loadCsv(const std::string& filename) {
  std::println("Filename:", filename);
  // NOTE: We accept `#` for comments
  std::vector<std::vector<std::string>> out;
  std::string line;
  std::ifstream file(filename);

  if (file.is_open()) {
    while (getline(file, line)) {
      if (line[0] != '#') {
        out.push_back(splitString(line, ','));
      }
    }

    file.close();
  }

  return out;
}
