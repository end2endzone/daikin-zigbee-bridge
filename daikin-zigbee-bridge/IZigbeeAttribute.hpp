#pragma once

#include <stdint.h>
#include "Arduino.h"

class IZigbeeAttribute
{
public:
  virtual ~IZigbeeAttribute() = default;

  virtual bool isInitialized() const = 0;

  virtual bool setup(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) = 0;
  virtual bool setup() = 0;

  virtual uint16_t getClusterId() const = 0;
  virtual uint16_t getAttributeId() const = 0;
  virtual uint8_t getEndpoint() const = 0;

  virtual bool readFromZigbee() = 0;
  virtual bool writeToZigbee() = 0;

  //virtual void* getDataPtr() = 0;
  virtual size_t getDataSize() const = 0;

  virtual void *newValue() const = 0;

  virtual String toString() const = 0;
};
