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
    T value = {0};
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

  bool setRaw(const T& value) {
    if (!isValid())
      return false; // unwrite
    bool written = setGenericAttributeRaw(&value, sizeof(T));
    return written;
  }

  virtual bool report() const {
    T value = {0};
    bool readed = get(value);
    if (!readed)
      return false;
    bool reported = reportAttribute(&value, sizeof(T));
    return reported;
  }

  bool setAndReport(const T& value) {
    if (!set(value))
      return false;
    bool reported = report();
    return reported;
  }

  bool setRawAndReport(const T& value) {
    if (!setRaw(value))
      return false;
    bool reported = report();
    return reported;
  }

  virtual size_t getSize() const override {
    return sizeof(T);
  }

  virtual void zbDataToHex(char* buffer, size_t buffer_size) const {
    T value = {0};
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

    String context;
    context += getName();
    context += "::";
    context += __FUNCTION__;

    // Assert correct size
    if (!zb_assert_attribute_size<T>(context.c_str(), _type_id))
      return false;

    // Assert correct sign
    if (!zb_assert_attribute_sign<T>(context.c_str(), _type_id))
      return false;
    
    return true;
  }
};
