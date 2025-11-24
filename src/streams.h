#pragma once

#include <print>
#include <vector>

struct Stream {
  std::string name;
  std::string url;
};

std::vector<Stream> loadStreams();
