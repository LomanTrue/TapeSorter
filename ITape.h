#pragma once

#include <cstdint>

class ITape {
public:
  virtual ~ITape() = default;
  virtual int32_t Read() = 0;
  virtual void Write(int32_t value) = 0;
  virtual bool MoveForward() = 0;
  virtual bool MoveBackward() = 0;
  virtual void Rewind() = 0;
  virtual bool AtEnd() const = 0;
  virtual bool AtBegin() const = 0;
};