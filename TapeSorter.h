#pragma once

#include "ITape.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

class TapeSorter {
public:
  using TempTapeFactory = std::function<std::unique_ptr<ITape>(std::size_t index, std::size_t length)>;

  TapeSorter(std::size_t chunkSize, TempTapeFactory tempFactory);

  void Sort(ITape& input, ITape& output);

private:
  std::size_t chunkSize_;
  TempTapeFactory tempFactory_;
};
