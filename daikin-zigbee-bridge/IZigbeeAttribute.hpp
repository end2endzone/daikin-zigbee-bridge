#pragma once

#include <stdint.h>
#include "Arduino.h"
#include <vector>

class IZigbeeAttribute
{
public:
  virtual ~IZigbeeAttribute() = default;

  virtual void init(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) = 0;
  virtual void init(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, uint16_t manuf_code) = 0;
  virtual bool isInitialized() const = 0;
  
  virtual bool setup() = 0;

  virtual bool isValid() const = 0;
  
  virtual const char * getName() const = 0;
  virtual uint8_t getEndpoint() const = 0;
  virtual uint16_t getClusterId() const = 0;
  virtual uint16_t getAttributeId() const = 0;
  virtual esp_zb_zcl_attr_type_t getTypeId() const = 0;
  virtual esp_zb_zcl_attr_access_t getAccessId() const = 0;
  virtual uint16_t getManufCode() const = 0;
  virtual size_t getSize() const = 0;

  virtual bool matches(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) const = 0;
  virtual bool report() const = 0;
  virtual bool update() = 0;
  virtual bool hasChanged() const = 0;

  virtual void *getDefaultDataPointer() = 0;

  virtual void notifyChange() const = 0;

  virtual String toString() const = 0;
};

// Define a type for handling list of attributes
typedef std::vector<IZigbeeAttribute*> ZigbeeAttributeList;

static IZigbeeAttribute* findAttribute(ZigbeeAttributeList& attributes, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) {
  for(size_t i=0; i<attributes.size(); i++) {
    IZigbeeAttribute* attr_p = attributes[i];
    if (attr_p->matches(endpoint, cluster_id, attr_id)) {
      return attr_p;
    }
  }
  return nullptr;
}
