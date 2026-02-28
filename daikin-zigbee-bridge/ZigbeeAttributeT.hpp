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
  void (*_on_value_change_callback)(T);
public:
  ZigbeeAttribute() : 
    ZigbeeAttributeBase(),
    _on_value_change_callback(nullptr) {
    memset(&_default_value, 0, sizeof(_default_value));
  }

  ZigbeeAttribute(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) :
    ZigbeeAttributeBase(endpoint, cluster_id, attr_id),
    _on_value_change_callback(nullptr) {
  }

  virtual ~ZigbeeAttribute() {}

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

  void onValueChange(void (*callback)(T)) {
    _on_value_change_callback = callback;
  }

  virtual void notifyChange() const {
    if (_on_value_change_callback && _data_p != nullptr) {
      // Copy the value from the internal pointer to prevent acquiring/releasing the lock.
      // Update notify have the potential to be executed from a zigbee callback
      // which should not acquiring/releasing the lock.

      // Since we are reading from _data_p (a pointer), make a copy to prevent unexpected changes during callbacks. 
      T copy = *((T*)_data_p);

      // Notify observers by calling their callbacks
      _on_value_change_callback(copy);
    }
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
