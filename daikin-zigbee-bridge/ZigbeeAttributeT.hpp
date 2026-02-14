#pragma once
#include "ZigbeeAttributeBase.hpp"
#include "zb_helper.h"
#include "zb_constants_helper.h"
#include "type_helper.h"

template <typename T>
class ZigbeeAttribute : public ZigbeeAttributeBase
{
//private:
//  T _value;
public:
  ZigbeeAttribute() : ZigbeeAttributeBase() {
  }

  ZigbeeAttribute(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) :
    ZigbeeAttributeBase(endpoint, cluster_id, attr_id) {
  }

  virtual bool setup()
  {
    bool result = ZigbeeAttributeBase::setup();
    if (!result)
      return false;

    // Assert attribute size
    if (_zbAttr == nullptr)
      return false;
    if (!checkAssertions())
      return false;

    return true;
  }

  virtual bool isInitialized() const override {
    if (!ZigbeeAttributeBase::isInitialized())
      return false;

    if (!checkAssertions())
      return false;

    return true;
  }

  T get() const {
    T value;
    bool readed = getGenericAttribute(&value, sizeof(T));
    return value;
  }

  bool get(T& value) const {
    bool readed = getGenericAttribute(&value, sizeof(T));
    return readed;
  }

  bool set(const T& value) {
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
  bool checkAssertions() const {
    size_t zb_type_size = zb_constants_zcl_attr_type_size((esp_zb_zcl_attr_type_t)_zbAttr->type);
    size_t template_size = sizeof(T);
    if (zb_type_size != template_size) {
      logEntry("Size assertion failed for attribute %s. Class type is %s", toString().c_str(), typeString<T>());
      return false;
    }
    return true;
  }
};
