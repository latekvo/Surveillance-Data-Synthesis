#include "csv.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> splitString(std::string& str, char sep) {
  std::stringstream ss(str);
  std::string temp;
  std::vector<std::string> out;

  while (getline(ss, temp, sep)) {
    out.push_back(temp);
  }

  return out;
}

std::vector<std::vector<std::string>> loadCsv(std::string filename) {
  std::println("debug:", filename);
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
