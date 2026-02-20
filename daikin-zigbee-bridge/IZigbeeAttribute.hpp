#pragma once

#include <stdint.h>
#include "Arduino.h"
#include <vector>

class IZigbeeAttribute
{
public:
  virtual ~IZigbeeAttribute() = default;

  virtual void init(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) = 0;
  virtual bool isInitialized() const = 0;
  
  virtual bool setup() = 0;

  virtual bool isValid() const = 0;
  
  virtual const char * getName() const = 0;
  virtual uint8_t getEndpoint() const = 0;
  virtual uint16_t getClusterId() const = 0;
  virtual uint16_t getAttributeId() const = 0;
  
  virtual bool matches(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) const = 0;

  virtual size_t getSize() const = 0;

  virtual void *getDefaultDataPointer() = 0;

  virtual String toString() const = 0;
};

typedef std::vector<IZigbeeAttribute*> ZigbeeAttributeList;

