#pragma once

#include <cstddef>
#include <fstream>
#include <string>

#include "ITape.h"
#include "TapeConfig.h"

class FileTape : public ITape {
  public:
  FileTape(const std::string& path, const TapeConfig& config);
  FileTape(const std::string& path, std::size_t length, const TapeConfig& config);

  ~FileTape() override;

  FileTape(const FileTape&) = delete;
  FileTape& operator=(const FileTape&) = delete;

  int32_t Read() override;
  void Write(int32_t value) override;
  bool MoveForward() override;
  bool MoveBackward() override;
  void Rewind() override;
  bool AtEnd() const override;
  bool AtBegin() const override;

  std::size_t length() const {return length_;}

private:
  void OpenExisting(const std::string& path);
  void CreateNew(const std::string& path, std::size_t length);
  void SeekToHead();

  std::string path_;
  TapeConfig config_;
  std::fstream stream_;
  std::size_t length_ = 0;
  std::size_t position_ = 0;
};