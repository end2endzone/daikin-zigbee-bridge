#pragma once

#include "SerialInterface.h"
#include <HardwareSerial.h>

class HardwareSerialAdapter : public SerialInterface
{
public:
  HardwareSerialAdapter(HardwareSerial &s) : serial(s) {}

  int available() override { return serial.available(); }
  int read() override { return serial.read(); }
  size_t write(uint8_t b) override { return serial.write(b); }
  size_t write(const uint8_t *data, size_t len) override { return serial.write(data, len); }

private:
  HardwareSerial &serial;
};
