#include "FileTape.h"

#include <cstdint>
#include <stdexcept>
#include <thread>
#include <vector>

constexpr std::size_t kElemSize = sizeof(int32_t);

void sleepFor(std::chrono::microseconds d) {
  if (d.count() > 0) {
    std::this_thread::sleep_for(d);
  }
}

FileTape::FileTape(const std::string& path, const TapeConfig& config) : path_(path), config_(config) {
  OpenExisting(path);
}

FileTape::FileTape(const std::string& path, std::size_t length, const TapeConfig& config)
  : path_(path), config_(config) {
  CreateNew(path, length);
}

FileTape::~FileTape() = default;

int32_t FileTape::Read() {
  if (AtEnd()) {
    throw std::runtime_error("FileTape::read: head is on the end");
  }
  sleepFor(config_.readDelay);

  int32_t value = 0;

  stream_.seekg(static_cast<std::streamoff>(position_ * kElemSize), std::ios::beg);
  stream_.read(reinterpret_cast<char*>(&value), kElemSize);
  if (!stream_) {
    throw std::runtime_error("FileTape::read: read failed at position " + std::to_string(position_));
  }
  return value;
}

void FileTape::Write(int32_t value) {
  if (AtEnd()) {
    throw std::runtime_error("FileTape::write: head is on the end");
  }
  sleepFor(config_.writeDelay);

  stream_.seekp(static_cast<std::streamoff>(position_ * kElemSize), std::ios::beg);
  stream_.write(reinterpret_cast<const char*>(&value), kElemSize);
  if (!stream_) {
    throw std::runtime_error("FileTape::write: write failed at position " + std::to_string(position_));
  }

  stream_.flush();
}

bool FileTape::MoveForward() {
  if (position_ >= length_) {
    return false;
  }
  sleepFor(config_.moveDelay);
  ++position_;
  return true;
}

bool FileTape::MoveBackward() {
  if (position_ == 0) {
    return false;
  }
  sleepFor(config_.moveDelay);
  --position_;
  return true;
}

void FileTape::Rewind() {
  if (position_ == 0) {
    return;
  }
  sleepFor(config_.rewindDelay);
  position_ = 0;
  SeekToHead();
}

bool FileTape::AtEnd() const {
  return position_ >= length_;
}

bool FileTape::AtBegin() const {
  return position_ == 0;
}

void FileTape::OpenExisting(const std::string& path) {
  stream_.open(path, std::ios::in | std::ios::out | std::ios::binary);
  if (!stream_.is_open()) {
    throw std::runtime_error("FileTape: cannot open file: " + path);
  }
  stream_.seekg(0, std::ios::end);
  const auto size = static_cast<std::streamoff>(stream_.tellg());
  if (size < 0 || static_cast<std::size_t>(size) % kElemSize != 0) {
    throw std::runtime_error("FileTape: file size is not a multiple of element size: " + path);
  }
  length_ = static_cast<std::size_t>(size) / kElemSize;
  position_ = 0;
  SeekToHead();
}

void FileTape::CreateNew(const std::string& path, std::size_t length) {
  std::ofstream creator(path, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!creator.is_open()) {
    throw std::runtime_error("FileTape: cannot create file: " + path);
  }

  if (length > 0) {
    constexpr std::size_t kBufElems = 4096;
    std::vector<int32_t> zeros(std::min(length, kBufElems), 0);
    std::size_t remaining = length;
    while (remaining > 0) {
      const std::size_t toWrite = std::min(remaining, zeros.size());
      creator.write(reinterpret_cast<const char*>(zeros.data()),
                    static_cast<std::streamsize>(toWrite * kElemSize));
      remaining -= toWrite;
    }
  }

  stream_.open(path, std::ios::in | std::ios::out | std::ios::binary);
  if (!stream_.is_open()) {
    throw std::runtime_error("FileTape: cannot reopen file after creation: " + path);
  }
  length_ = length;
  position_ = 0;
  SeekToHead();
}

void FileTape::SeekToHead() {
  const auto offset = static_cast<std::streamoff>(position_ * kElemSize);
  stream_.seekg(offset, std::ios::beg);
  stream_.seekp(offset, std::ios::beg);
}