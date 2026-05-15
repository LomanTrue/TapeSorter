#include "TapeSorter.h"

#include <algorithm>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

TapeSorter::TapeSorter(std::size_t chunkSize, TempTapeFactory tempFactory)
  : chunkSize_(chunkSize), tempFactory_(std::move(tempFactory)) {
  if (chunkSize_ == 0) {
    throw std::invalid_argument("TapeSorter: chunkSize must be >= 1");
  }
  if (!tempFactory_) {
    throw std::invalid_argument("TapeSorter: tempFactory must not be null");
  }
}

void TapeSorter::Sort(ITape& input, ITape& output) {
  input.Rewind();

  std::vector<std::unique_ptr<ITape>> runs;
  std::vector<int32_t> buffer;
  buffer.reserve(chunkSize_);

  while (!input.AtEnd()) {
    buffer.clear();
    while (buffer.size() < chunkSize_ && !input.AtEnd()) {
      buffer.push_back(input.Read());
      input.MoveForward();
    }
    if (buffer.empty()) {
      break;
    }

    std::sort(buffer.begin(), buffer.end());

    auto run = tempFactory_(runs.size(), buffer.size());
    run->Rewind();
    for (int32_t v : buffer) {
      run->Write(v);
      run->MoveForward();
    }
    runs.push_back(std::move(run));
  }

  output.Rewind();

  if (runs.empty()) {
    return;
  }

  for (auto& run : runs) {
    run->Rewind();
  }

  using HeapItem = std::pair<int32_t, std::size_t>;
  auto cmp = [](const HeapItem& a, const HeapItem& b) {
    if (a.first != b.first) {
      return a.first > b.first;
    }
    return a.second > b.second;
  };
  std::priority_queue<HeapItem, std::vector<HeapItem>, decltype(cmp)> heap(cmp);

  for (std::size_t i = 0; i < runs.size(); ++i) {
    if (!runs[i]->AtEnd()) {
      heap.emplace(runs[i]->Read(), i);
    }
  }

  while (!heap.empty()) {
    const auto [value, run_index] = heap.top();
    heap.pop();

    if (output.AtEnd()) {
      throw std::runtime_error("TapeSorter: output tape is shorter than input");
    }
    output.Write(value);
    output.MoveForward();

    ITape& run = *runs[run_index];
    run.MoveForward();
    if (!run.AtEnd()) {
      heap.emplace(run.Read(), run_index);
    }
  }
}