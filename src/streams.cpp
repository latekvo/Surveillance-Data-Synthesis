#include "streams.h"

#include <print>
#include <stdexcept>
#include <vector>

#include "consts.h"
#include "csv.h"

std::vector<Stream> loadStreams() {
  std::vector rows = loadCsv(STREAMS_FILE);

  if (rows.size() == 0) {
    throw std::runtime_error(
        "Error - No streams provided! Add at least one entry to "
        "'streams.csv'.");
  }

  std::vector<Stream> streams;

  for (const std::vector<std::string>& row : rows) {
    if (row.size() != 2) {
      throw std::runtime_error("Error - The streams CSV file is malformed!");
    }

    std::println("Loading stream: {} {}", row[0], row[1]);
    streams.push_back(Stream{row[0], row[1]});
  }

  return streams;
}
