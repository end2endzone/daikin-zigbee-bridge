#pragma once
#include "SerialInterface.h"

class MockSerial : public SerialInterface
{
private:
  // RX
  uint8_t * rxBuffer;
  size_t    rxSize;
  size_t    rxPos;

  // TX
  uint8_t * txBuffer;
  size_t    txSize;
  size_t    txPos;

public:
  MockSerial() :
    rxBuffer(nullptr), rxSize(0), rxPos(0),
    txBuffer(nullptr), txSize(0), txPos(0)
  {
  }

  ~MockSerial()
  {
    clear();
  }

  // ---------------------------------------------------------
  // RX BUFFER
  // ---------------------------------------------------------
  int available() const
  {
    return (rxSize > rxPos) ? (int)(rxSize - rxPos) : 0;
  }

  int read()
  {
    if (available() == 0)
      return -1;

    int result = rxBuffer[rxPos];
    rxPos++;
    return result;
  }

  // Load data into RX buffer (replaces existing content)
  size_t loadRx(const uint8_t *data, size_t len)
  {
    if (!reallocRxCapacity(len)) return 0;
    memcpy(&rxBuffer[rxPos], data, len);
    rxPos = len;
    return len;
  }

  // ---------------------------------------------------------
  // TX BUFFER
  // ---------------------------------------------------------
  size_t write(uint8_t b)
  {
    if (!reallocTxCapacity(txPos + 1)) return 0;
    txBuffer[txPos] = b;
    txPos++;
    return 1;
  }

  size_t write(const uint8_t *data, size_t len)
  {
    if (!reallocTxCapacity(txPos + len)) return 0;
    memcpy(&txBuffer[txPos], data, len);
    txPos += len;
    return len;
  }

  // Retrieve RX buffer for assertions
  const uint8_t *getRxBuffer() const { return rxBuffer; }
  size_t getRxLength() const { return rxPos; }

  // Retrieve TX buffer for assertions
  const uint8_t *getTxBuffer() const { return txBuffer; }
  size_t getTxLength() const { return txPos; }

  // ---------------------------------------------------------
  // Copy TX to RX
  // ---------------------------------------------------------
  void copyTxToRx()
  {
    loadRx(txBuffer, txPos);
  }

  // ---------------------------------------------------------
  // Reset everything
  // ---------------------------------------------------------
  void clear()
  {
    rxSize = 0;
    rxPos = 0;
    txPos = 0;

    if (rxBuffer) free(rxBuffer);
    if (txBuffer) free(txBuffer);
    rxBuffer = nullptr;
    txBuffer = nullptr;
  }

private:
  // ---------------------------------------------------------
  // Ensure RX buffer has enough capacity
  // ---------------------------------------------------------
  bool reallocRxCapacity(size_t required)
  {
    // Check if allocated size is already big enough
    if (required <= rxSize)
      return true;

    // Compute new size requirement
    size_t newSize = (rxSize == 0) ? 16 : rxSize;
    while (newSize < required)
      newSize *= 2;

    // Try to allocate
    uint8_t * newBuffer = (uint8_t *)realloc(rxBuffer, newSize);
    if (!newBuffer)
      return false;

    // Reallocation success, existing internal pointers & values
    rxBuffer = newBuffer;
    rxSize = newSize;

    return true;
  }

  // ---------------------------------------------------------
  // Ensure TX buffer has enough capacity
  // ---------------------------------------------------------
  bool reallocTxCapacity(size_t required)
  {
    // Check if allocated size is already big enough
    if (required <= txSize)
      return true;

    // Compute new size requirement
    size_t newSize = (txSize == 0) ? 16 : txSize;
    while (newSize < required)
      newSize *= 2;

    // Try to allocate
    uint8_t * newBuffer = (uint8_t *)realloc(txBuffer, newSize);
    if (!newBuffer)
      return false;

    // Reallocation success, existing internal pointers & values
    txBuffer = newBuffer;
    txSize = newSize;

    return true;
  }
};
