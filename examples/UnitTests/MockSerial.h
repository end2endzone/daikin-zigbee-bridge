#pragma once
#include "SerialInterface.h"
#include <vector>

class MockSerial : public SerialInterface
{
public:
  std::vector<uint8_t> rxBuffer;
  std::vector<uint8_t> txBuffer;

  void pushRx(const uint8_t *data, size_t len)
  {
    rxBuffer.insert(rxBuffer.end(), data, data + len);
  }

  int available() const override
  {
    return rxBuffer.size();
  }

  int read() override
  {
    if (rxBuffer.empty())
      return -1;
    uint8_t b = rxBuffer.front();
    rxBuffer.erase(rxBuffer.begin());
    return b;
  }

  size_t write(uint8_t b) override
  {
    txBuffer.push_back(b);
    return 1;
  }

  size_t write(const uint8_t *data, size_t len) override
  {
    txBuffer.insert(txBuffer.end(), data, data + len);
    return len;
  }
};
