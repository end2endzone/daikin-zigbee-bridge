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
  uint8_t _endpoint;
  uint16_t _cluster_id;
  uint16_t _attr_id;
  esp_zb_zcl_attr_type_t _type_id; // uint8_t
  esp_zb_zcl_attr_access_t _access_id; // uint8_t
  uint16_t _manuf_code;

public:
  ZigbeeAttributeBase()
    : _cluster_id(0),
      _attr_id(0),
      _endpoint(0),
      _type_id(ESP_ZB_ZCL_ATTR_TYPE_INVALID),
      _access_id((esp_zb_zcl_attr_access_t)0),
      _manuf_code(0)
      {
  }

  ZigbeeAttributeBase(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id)
    : _endpoint(endpoint),
      _cluster_id(cluster_id),
      _attr_id(attr_id),
      _type_id(ESP_ZB_ZCL_ATTR_TYPE_INVALID),
      _access_id((esp_zb_zcl_attr_access_t)0),
      _manuf_code(0)
      {
  }

  virtual ~ZigbeeAttributeBase() = default;

private:
  inline bool lookupAttribute() {
    // Prevent crashing the system if Zigbee stack is not started.
    // Calling esp_zb_lock_acquire() before the stack is ready will crash the 
    // ESP32 with error: "Zigbee lock is not ready".
    if (!esp_zb_is_started()) {
      return false;
    }

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

    // Keep the attribute's type and access
    _type_id = (esp_zb_zcl_attr_type_t)attr->type;
    _access_id = (esp_zb_zcl_attr_access_t)attr->access;
    _manuf_code = attr->manuf_code;

  unlock_and_return:
    esp_zb_lock_release();
    return isValid();
  }

public:
  virtual bool setup() {
    if (!isInitialized())
      return false;
    return lookupAttribute();
  }

  virtual bool init(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) {
    _endpoint = endpoint;
    _cluster_id = cluster_id;
    _attr_id = attr_id;
  }

  virtual bool isInitialized() const override {
    if (_endpoint == 0 && _cluster_id == 0 && _attr_id == 0) return false;
    return true;
  }

  virtual uint8_t getEndpoint() const override { return _endpoint; }
  virtual uint16_t getClusterId() const override { return _cluster_id; }
  virtual uint16_t getAttributeId() const override { return _attr_id; }

  bool inline matches(uint16_t cluster_id, uint16_t attr_id) { return (cluster_id == _cluster_id && attr_id == _attr_id); }

  inline size_t zbTypeSize() const {
    size_t size = zb_constants_zcl_attr_type_size(_type_id);
    return size;
  }

  inline TypeSign zbTypeSign() const {
    TypeSign sign = zb_constants_zcl_attr_type_signed(_type_id);
    return sign;
  }

  inline const char * zbTypeName() const {
    const char * name = zb_constants_zcl_attr_type_to_string(_type_id);
    return name;
  }

  inline const char * zbClusterName() const {
    const char * name = zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)_cluster_id);
    return name;
  }

  inline const char * zbAttributeName() const {
    const char * name = zb_constants_smart_cluster_attr_to_string((esp_zb_zcl_cluster_id_t)_cluster_id, _attr_id);
    return name;
  }

  virtual String toString() const override {    
    const char* cluster_name = zbClusterName();
    const char* attr_name = zbAttributeName();
    const char * attr_type_name = zbTypeName();
    size_t attr_type_size = zbTypeSize();

    return format("{endpoint: 0x%02x, attr: %s (0x%04x), cluster: %s (0x%04x), type: %s, size: %d}",
      _endpoint, attr_name, _attr_id, cluster_name, _cluster_id, attr_type_name, attr_type_size);
  }

  virtual bool isValid() const {
    if (_endpoint == 0 && _cluster_id == 0 && _attr_id == 0) return false; // not initialized
    if (_type_id == ESP_ZB_ZCL_ATTR_TYPE_INVALID || _access_id == 0) return false; // not registered, call setup() again
    return true;
  }

  bool getGenericAttribute(void* output_ptr, size_t output_size) const {
    return false;
  //  if (output_ptr == nullptr || output_size == 0)
  //    return false;
  //  if (!isValid())
  //    return false;
  //
  //  bool success = false;
  //  esp_zb_lock_acquire(portMAX_DELAY);
  //  
  //  // get attribute descriptor
  //  esp_zb_zcl_attr_t * attr = esp_zb_zcl_get_attribute(
  //    _endpoint,
  //    _cluster_id,
  //    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
  //    _attr_id);
  //  if (attr == nullptr) {
  //    logEntry("Failed to read attribute %s", toString().c_str());
  //    goto unlock_and_return;
  //  }
  //
  //  // Read/Copy the value
  //  memcpy(output_ptr, attr->data_p, output_size);
  //  success = true;
  //
  //unlock_and_return:
  //  esp_zb_lock_release();
  //  return success;
  }

  bool setGenericAttribute(const void* value_ptr, size_t value_size) const {
    return false;
    
  //  if (value_ptr == nullptr || value_size == 0)
  //    return false;
  //  if (!isValid())
  //    return false;
  //
  //  esp_zb_zcl_status_t status = ESP_ZB_ZCL_STATUS_SUCCESS;
  //  esp_zb_lock_acquire(portMAX_DELAY);
  //
  //  // set attribute value
  //  status = esp_zb_zcl_set_attribute_val(
  //    _endpoint,
  //    _cluster_id,
  //    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
  //    _attr_id,
  //    (void*)value_ptr,
  //    false
  //  );
  //  if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
  //    logEntry("Failed to write attribute %s. Status: 0x%x: (%s)", toString().c_str(), status, esp_zb_zcl_status_to_name(status));
  //  }
  //
  //  esp_zb_lock_release();
  //  return (status == ESP_ZB_ZCL_STATUS_SUCCESS);
  }
};
