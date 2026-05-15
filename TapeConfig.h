#pragma once

#include <chrono>
#include <string>

struct TapeConfig {
  std::chrono::microseconds readDelay{0};
  std::chrono::microseconds writeDelay{0};
  std::chrono::microseconds moveDelay{0};
  std::chrono::microseconds rewindDelay{0};

  static TapeConfig LoadFromFile(const std::string& path);
};
