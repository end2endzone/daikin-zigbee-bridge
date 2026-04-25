#pragma once
#include "SerialInterface.h"
#include <vector>

class MockSerial : public SerialInterface
{
private:
  std::vector<uint8_t> rxBuffer;
  std::vector<uint8_t> txBuffer;

public:
  size_t getRxSize() const {
    return rxBuffer.size();
  }

  size_t getTxSize() const {
    return txBuffer.size();
  }

  void copyTxToRxBuffer()
  {
    rxBuffer = txBuffer;
  }

  bool corruptRxByte(size_t index, uint8_t b)
  {
    if (index >= rxBuffer.size())
      return false;
    rxBuffer[index] = b;
    return true;
  }

  bool corruptTxByte(size_t index, uint8_t b)
  {
    if (index >= txBuffer.size())
      return false;
    txBuffer[index] = b;
    return true;
  }

  bool dropRxByte(size_t index)
  {
    if (index >= rxBuffer.size())
      return false;
    rxBuffer.erase(rxBuffer.begin() + index);
    return true;
  }

  bool dropTxByte(size_t index)
  {
    if (index >= txBuffer.size())
      return false;
    txBuffer.erase(txBuffer.begin() + index);
    return true;
  }

  bool dropLastRxByte()
  {
    return dropRxByte(rxBuffer.size() - 1);
  }

  bool dropLastTxByte()
  {
    return dropTxByte(txBuffer.size() - 1);
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
