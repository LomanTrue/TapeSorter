#include "TapeConfig.h"

#include <fstream>
#include <sstream>
#include <string>

std::string trim(const std::string& s) {
  const auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }
  const auto last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

TapeConfig TapeConfig::LoadFromFile(const std::string& path) {
  TapeConfig config;

  std::ifstream file(path);
  if (!file.is_open()) {
    return config;
  }

  std::string line;
  while (std::getline(file, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    const auto eq = line.find('=');
    if (eq == std::string::npos) {
      continue;
    }

    const std::string key = trim(line.substr(0, eq));
    const std::string value = trim(line.substr(eq + 1));

    long long parsed = 0;
    try {
      parsed = std::stoll(value);
    } catch (...) {
      continue;
    }
    if (parsed < 0) {
      parsed = 0;
    }

    const auto us = std::chrono::microseconds(parsed);
    if (key == "read_delay_us") {
      config.readDelay = us;
    } else if (key == "write_delay_us") {
      config.writeDelay = us;
    } else if (key == "move_delay_us") {
      config.moveDelay = us;
    } else if (key == "rewind_delay_us") {
      config.rewindDelay = us;
    }
  }

  return config;
}