#pragma once

#include "IZigbeeAttribute.hpp"
#include "esp_zigbee_type.h"
#include "zb_helper.h"
#include "zb_constants_helper.h"
#include "logging.h"
#include "format_helper.h"
#include "scope_debugger.h"

class ZigbeeAttributeBase : public IZigbeeAttribute
{
protected:
  bool _initialized;
  uint8_t _endpoint;
  uint16_t _cluster_id;
  uint16_t _attr_id;
  esp_zb_zcl_attr_t* _zbAttr;

public:
  ZigbeeAttributeBase()
    : _initialized(false),
      _cluster_id(0),
      _attr_id(0),
      _endpoint(0),
      _zbAttr(nullptr) {
  }

  ZigbeeAttributeBase(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id)
    : _initialized(false),
      _endpoint(endpoint),
      _cluster_id(cluster_id),
      _attr_id(attr_id),
      _zbAttr(nullptr) {
  }

  virtual ~ZigbeeAttributeBase() = default;

  virtual bool isInitialized() const override {
    return _initialized;
  }

private:
  bool setupInternal() {
    _initialized = false;

    esp_zb_lock_acquire(portMAX_DELAY);
    
    // get attribute descriptor
    esp_zb_zcl_attr_t * attr = esp_zb_zcl_get_attribute(
      _endpoint,
      _cluster_id,
      ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
      _attr_id);
    if (attr == nullptr) {
      logEntry("Failed to setup attribute %s", toString().c_str());
      goto unlock_and_return;
    }

    // Keep the attr
    _zbAttr = attr;

  unlock_and_return:
    esp_zb_lock_release();
    _initialized = isValid(); // update _initialized
    return _initialized;
  }

public:
  virtual bool setup() {
    return setupInternal();
  }

  virtual bool setup(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) {
    _endpoint = endpoint;
    _cluster_id = cluster_id;
    _attr_id = attr_id;

    return setupInternal();
  }

  virtual uint8_t getEndpoint() const override { return _endpoint; }
  virtual uint16_t getClusterId() const override { return _cluster_id; }
  virtual uint16_t getAttributeId() const override { return _attr_id; }

  virtual String toString() const override {
    // Compute data pointer
    void* dataPtr = getSafeDataPointer();

    // Convert data pointer content to hex
    char value_hex[32] = {0};
    value_hex[0] = '0';
    value_hex[1] = 'x';
    toHex(dataPtr, size(), &value_hex[2]);
    if (value_hex[2] == '\0') { // if toHex() returned an empty string
      // NULL output
      value_hex[0] = 'N';
      value_hex[1] = 'U';
      value_hex[2] = 'L';
      value_hex[3] = 'L';
      value_hex[4] = '\0';
    }
    
    const char * attr_type_desc = getSafeZigbeeAttrTypeDescritor();

    esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)_cluster_id;
    const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
    const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, _attr_id);

    return format("{endpoint: 0x%02x, attr: 0x%04x (%s), cluster: 0x%04x (%s), type: %s, size: %d, value: %s}", _endpoint, _attr_id, attr_name, _cluster_id, cluster_name, attr_type_desc, size(), value_hex);
  }

protected:
  bool isValid() const {
    bool valid = (
        !(_endpoint == 0 && _cluster_id == 0 && _attr_id == 0) &&
        _zbAttr != nullptr);
    return valid;
  }

  bool getGenericAttribute(void* output_ptr, size_t output_size) const {
    if (output_ptr == nullptr || output_size == 0)
      return false;
    if (_zbAttr == nullptr)
      return false;

    esp_zb_lock_acquire(portMAX_DELAY);
    
    // Read the value
    memcpy(output_ptr, _zbAttr->data_p, output_size);

    esp_zb_lock_release();
    return true;
  }

  bool setGenericAttribute(const void* value_ptr, size_t value_size) const {
    if (value_ptr == nullptr || value_size == 0)
      return false;
    if (_zbAttr == nullptr)
      return false;

    bool ret = true;
    esp_zb_zcl_status_t status = ESP_ZB_ZCL_STATUS_SUCCESS;
    esp_zb_lock_acquire(portMAX_DELAY);

    // set attribute value
    status = esp_zb_zcl_set_attribute_val(
      _endpoint,
      _cluster_id,
      ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
      _attr_id,
      (void*)value_ptr,
      false
    );
    if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
      logEntry("Failed to write attribute %s. Status: 0x%x: (%s)", toString().c_str(), status, esp_zb_zcl_status_to_name(status));
      ret = false;
    }

    esp_zb_lock_release();
    return ret;
  }

  size_t getSafeZigbeeAttrSize() const {
    size_t attr_size = 0;
    if (_zbAttr != nullptr)
      attr_size = zb_constants_zcl_attr_type_size((esp_zb_zcl_attr_type_t)_zbAttr->type);
    return attr_size;
  }

  TypeSign getSafeZigbeeAttrTypeSign() const {
    TypeSign attr_sign = TYPE_SIGN_UNKNOWN;
    if (_zbAttr != nullptr)
      attr_sign = zb_constants_zcl_attr_type_signed((esp_zb_zcl_attr_type_t)_zbAttr->type);
    return attr_sign;
  }

  const char * getSafeZigbeeAttrTypeDescritor() const {
    const char * desc = "NULL";
    if (_zbAttr != nullptr)
      desc = zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)_zbAttr->type);
    return desc;
  }

  void * getSafeDataPointer() const {
    void * ptr = nullptr;
    if (_zbAttr != nullptr)
      ptr = _zbAttr->data_p;
    return ptr;
  }
};
