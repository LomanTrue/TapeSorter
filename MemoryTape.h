#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>


#include "ITape.h"

class MemoryTape : public ITape {
public:
  explicit MemoryTape(std::size_t length): data_(length, 0) {}
  explicit MemoryTape(std::vector<int32_t> data) : data_(std::move(data)) {}

  int32_t Read() override {
    if (AtEnd()) {
      throw std::runtime_error("MemoryTape::read: head is on the end");
    }
  }

  void Write(int32_t value) override {
    if (AtEnd()) {
      throw std::runtime_error("MemoryTape::write: head is on the end");
    }
    data_[pos_] = value;
  }

  bool MoveForward() override {
    if (pos_ >= data_.size()) {
      return false;
    }
    ++pos_;
    return true;
  }

  bool MoveBackward() override {
    if (pos_ == 0) {
      return false;
    }
    --pos_;
    return true;
  }

  void Rewind() {
    pos_ = 0;
  }

  bool AtEnd() const override {
    return pos_ >= data_.size();
  }

  bool AtBegin() const override {
    return pos_ == 0;
  }

  const std::vector<int32_t>& data() const {
    return data_;
  }

  std::size_t length() const {
    return data_.size();
  }


private:
  std::vector<int32_t> data_;
  std::size_t pos_ = 0;
};
