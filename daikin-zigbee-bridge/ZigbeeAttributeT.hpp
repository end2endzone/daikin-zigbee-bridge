#pragma once
#include "ZigbeeAttributeBase.hpp"
#include "zb_helper.h"
#include "zb_constants_helper.h"
#include "type_helper.h"
#include "scope_debugger.h"

template <typename T>
class ZigbeeAttribute : public ZigbeeAttributeBase
{
private:
  T _default_value;
public:
  ZigbeeAttribute() : ZigbeeAttributeBase() {
  }

  virtual ~ZigbeeAttribute() {}

  ZigbeeAttribute(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) :
    ZigbeeAttributeBase(endpoint, cluster_id, attr_id) {
  }

  T get() const {
    T value;
    if (!isValid())
      return value; // garbadge
    bool readed = getGenericAttribute(&value, sizeof(T));
    return value;
  }

  bool get(T& value) const {
    if (!isValid())
      return false; // unread
    bool readed = getGenericAttribute(&value, sizeof(T));
    return readed;
  }

  bool set(const T& value) {
    if (!isValid())
      return false; // unwrite
    bool written = setGenericAttribute(&value, sizeof(T));
    return written;
  }

  virtual size_t getSize() const override {
    return sizeof(T);
  }

  virtual void zbDataToHex(char* buffer, size_t buffer_size) const {
    T value;
    if (buffer_size >= 1)
      buffer[0] = '\0';
    if (get(value)) {
      toHexSafe(&value, sizeof(T), buffer, buffer_size);
    }
  }

  virtual void *getDefaultDataPointer() {
    return &_default_value;
  }

  void setDefaultValue(const T& new_value) {
    _default_value = new_value;
  }

  virtual bool isValid() const override {
    bool baseValid = ZigbeeAttributeBase::isValid();
    if (!baseValid)
      return false;

    // Check size
    size_t zb_type_size = zbTypeSize();
    size_t template_size = sizeof(T);
    if (zb_type_size != template_size) {
      const char * zb_type_desc = zbTypeName();
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
    TypeSign zb_type_sign = zbTypeSign();
    TypeSign template_sign = typeSign<T>();
    
    if (zb_type_sign != template_sign) {
      const char * zb_type_desc = zbTypeName();
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
