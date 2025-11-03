#include "list_parser.h"

#include <fstream>
#include <string>
#include <vector>

std::vector<std::string> parseListFile(std::string filename) {
  std::vector<std::string> out;
  std::string line;
  std::ifstream file(filename);

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
