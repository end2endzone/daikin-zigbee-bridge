#pragma once
#include "ZigbeeAttributeBase.hpp"
#include "zb_helper.h"
#include "zb_constants_helper.h"
#include "type_helper.h"

template <typename T>
class ZigbeeAttribute : public ZigbeeAttributeBase
{
public:
  ZigbeeAttribute() : ZigbeeAttributeBase() {
  }

  ZigbeeAttribute(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) :
    ZigbeeAttributeBase(endpoint, cluster_id, attr_id) {
  }

  virtual bool setup()
  {
    bool success = ZigbeeAttributeBase::setup();
    _initialized = false; // force uninitialized again
    if (!success)
      return false;

    // Assert attribute size
    if (_zbAttr == nullptr)
      return false;
    if (!isValidationOK())
      return false;

    checkInit(); // update _initialized
    return true;
  }

  T get() const {
    T value;
    if (!isInitialized())
      return value; // garbadge
    bool readed = getGenericAttribute(&value, sizeof(T));
    return value;
  }

  bool get(T& value) const {
    if (!isInitialized())
      return false; // unread
    bool readed = getGenericAttribute(&value, sizeof(T));
    return readed;
  }

  bool set(const T& value) {
    if (!isInitialized())
      return false; // unwrite
    bool written = setGenericAttribute(&value, sizeof(T));
    return written;
  }

  virtual size_t size() const override {
    return sizeof(T);
  }

  virtual void *newValue() const {
    return new T;
  }

protected:
  bool isValidationOK() const {
    // Note: Can't check isInitialized() here since
    // isValidationOK() is called during setup().

    // Check size
    size_t zb_type_size = getSafeZigbeeAttrSize();
    size_t template_size = sizeof(T);
    if (zb_type_size != template_size) {
      const char * zb_type_desc = getSafeZigbeeAttrTypeDescritor();
      const char * class_name = typeString<T>();
      logEntry("*** Size assertion failed for attribute %s ! Zigbee type '%s' size (%d bit) does not match class type '%s' size (%d bit).",
        toString().c_str(),
        zb_type_desc,
        zb_type_size*8,
        class_name,
        template_size*8
        );
      return false;
    }

    // Check signed
    TypeSign zb_type_sign = getSafeZigbeeAttrTypeSign();
    TypeSign template_sign = typeSign<T>();
    
    if (zb_type_sign != template_sign) {
      const char * zb_type_desc = getSafeZigbeeAttrTypeDescritor();
      const char * class_name = typeString<T>();
      logEntry("*** Sign assertion failed for attribute %s ! Zigbee type '%s' sign (%s) does not match class type '%s' sign (%s).",
        toString().c_str(),
        zb_type_desc,
        ::toString(zb_type_sign),
        class_name,
        ::toString(template_sign)
        );
      return false;
    }
    
    return true;
  }
};
