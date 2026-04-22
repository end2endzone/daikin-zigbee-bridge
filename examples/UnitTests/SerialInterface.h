#pragma once

#include <stdint.h>
#include <stddef.h>

class SerialInterface
{
public:
  virtual int available() = 0;
  virtual int read() = 0;
  virtual size_t write(uint8_t b) = 0;
  virtual size_t write(const uint8_t *data, size_t len) = 0;
  virtual ~SerialInterface() {}
};
