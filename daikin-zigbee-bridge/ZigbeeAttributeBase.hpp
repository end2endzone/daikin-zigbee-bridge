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
  const char * _name;
  uint8_t _endpoint;
  uint16_t _cluster_id;
  uint16_t _attr_id;
  esp_zb_zcl_attr_type_t _type_id; // uint8_t
  esp_zb_zcl_attr_access_t _access_id; // uint8_t
  uint16_t _manuf_code;
  void * _data_p;
  bool _is_manuf_specific;

public:
  ZigbeeAttributeBase()
    : _name(nullptr),
      _endpoint(0),
      _cluster_id(0),
      _attr_id(0),
      _type_id(ESP_ZB_ZCL_ATTR_TYPE_INVALID),
      _access_id((esp_zb_zcl_attr_access_t)0),
      _manuf_code(0),
      _data_p(nullptr),
      _is_manuf_specific(false)
      {
  }

  ZigbeeAttributeBase(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id)
    : _name(name),
      _endpoint(endpoint),
      _cluster_id(cluster_id),
      _attr_id(attr_id),
      _type_id(ESP_ZB_ZCL_ATTR_TYPE_INVALID),
      _access_id((esp_zb_zcl_attr_access_t)0),
      _manuf_code(0),
      _data_p(nullptr),
      _is_manuf_specific(false)
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
      // Try again with manufacturer specific APIs...
      attr = esp_zb_zcl_get_manufacturer_attribute(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id,
        _manuf_code);      
    }
    if (attr == nullptr) {
      logEntry("WARNING: Attribute lookup fail: %s", toString().c_str());
      goto unlock_and_return;
    }

    // Keep the attribute's type and access
    _type_id = (esp_zb_zcl_attr_type_t)attr->type;
    _access_id = (esp_zb_zcl_attr_access_t)attr->access;
    _manuf_code = attr->manuf_code;

    // Keep a pointer to the actual data to be able to read/write the attribute
    // without having to acquire/releasing the lock.
    _data_p = attr->data_p;

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

  virtual void init(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) {
    _name = name;
    _endpoint = endpoint;
    _cluster_id = cluster_id;
    _attr_id = attr_id;
  }

  virtual void init(const char * name, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, uint16_t manuf_code) {
    _name = name;
    _endpoint = endpoint;
    _cluster_id = cluster_id;
    _attr_id = attr_id;
    _manuf_code = manuf_code;

    _is_manuf_specific = true;
  }

  virtual bool isInitialized() const override {
    if (_name == nullptr && _endpoint == 0 && _cluster_id == 0 && _attr_id == 0 && _manuf_code != 0) return false;
    return true;
  }

  virtual const char * getName() const override { return _name; }
  virtual uint8_t getEndpoint() const override { return _endpoint; }
  virtual uint16_t getClusterId() const override { return _cluster_id; }
  virtual uint16_t getAttributeId() const override { return _attr_id; }
  virtual esp_zb_zcl_attr_type_t getTypeId() const override { return _type_id; }
  virtual esp_zb_zcl_attr_access_t getAccessId() const override { return _access_id; }
  virtual uint16_t getManufCode() const override { return _manuf_code; }

  bool inline matches(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id) const { return (cluster_id == _cluster_id && attr_id == _attr_id); }

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

  inline const char * zbAccessName() const {
    const char * name = zb_constants_zcl_attr_access_to_string((esp_zb_zcl_attr_access_t)_access_id);
    return name;
  }

  virtual String toString() const override {    
    const char* cluster_name = zbClusterName();
    const char* attr_name = zbAttributeName();
    const char * attr_type_name = zbTypeName();
    size_t attr_type_size = zbTypeSize();
    const char * access_name = zbAccessName();

    return format("{name: '%s', endpoint: 0x%02x, attr: %s (0x%04x), cluster: %s (0x%04x), type: %s, access: %s (0x%04x), size: %d}",
      _name, _endpoint, attr_name, _attr_id, cluster_name, _cluster_id, attr_type_name, access_name, _access_id, attr_type_size);
  }

  virtual bool isValid() const {
    if (_name == 0 && _endpoint == 0 && _cluster_id == 0 && _attr_id == 0) return false; // not initialized
    if (_type_id == ESP_ZB_ZCL_ATTR_TYPE_INVALID || _access_id == 0) return false; // not registered, call setup() again
    return true;
  }

protected:
  bool getGenericAttribute(void* output_ptr, size_t output_size) const {
    if (output_ptr == nullptr || output_size == 0)
      return false;
    if (!isValid())
      return false;
  
    bool success = false;
    esp_zb_lock_acquire(portMAX_DELAY);
    
    // get attribute descriptor
    esp_zb_zcl_attr_t * attr = nullptr;
    if (!_is_manuf_specific) {
      attr = esp_zb_zcl_get_attribute(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id);
    } else {
      attr = esp_zb_zcl_get_manufacturer_attribute(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id,
        _manuf_code);      
    }
    if (attr == nullptr) {
      logEntry("WARNING: Failed to read attribute %s", toString().c_str());
      goto unlock_and_return;
    }
  
    // Read/Copy the value
    memcpy(output_ptr, attr->data_p, output_size);
    success = true;
  
  unlock_and_return:
    esp_zb_lock_release();
    return success;
  }

  bool setGenericAttribute(const void* value_ptr, size_t value_size) const {
    if (value_ptr == nullptr || value_size == 0)
      return false;
    if (!isValid())
      return false;
  
    esp_zb_zcl_status_t status = ESP_ZB_ZCL_STATUS_SUCCESS;
    esp_zb_lock_acquire(portMAX_DELAY);
  
    // set attribute value
    esp_zb_zcl_attr_t * attr = nullptr;
    if (!_is_manuf_specific) {
      status = esp_zb_zcl_set_attribute_val(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id,
        (void*)value_ptr,
        false
      );
    } else {
      status = esp_zb_zcl_set_manufacturer_attribute_val(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _manuf_code,
        _attr_id,
        (void*)value_ptr,
        false
      );
    }
    if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
      logEntry("WARNING: Failed to write attribute %s. Status: 0x%x: (%s)", toString().c_str(), status, esp_zb_zcl_status_to_name(status));
    }

    esp_zb_lock_release();
    return (status == ESP_ZB_ZCL_STATUS_SUCCESS);
  }

  bool setGenericAttributeRaw(const void* value_ptr, size_t value_size) const {
    if (value_ptr == nullptr || value_size == 0)
      return false;
    if (!isValid())
      return false;
  
    bool success = false;
    esp_zb_lock_acquire(portMAX_DELAY);
    
    // get attribute descriptor
    esp_zb_zcl_attr_t * attr = nullptr;
    if (!_is_manuf_specific) {
      attr = esp_zb_zcl_get_attribute(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id);
    } else {
      attr = esp_zb_zcl_get_manufacturer_attribute(
        _endpoint,
        _cluster_id,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        _attr_id,
        _manuf_code);      
    }
    if (attr == nullptr) {
      logEntry("WARNING: Failed to write attribute raw value %s", toString().c_str());
      goto unlock_and_return;
    }
  
    // Write the value
    memcpy(attr->data_p, value_ptr, value_size);
    success = true;
  
  unlock_and_return:
    esp_zb_lock_release();
    return success;
  }

  bool reportAttribute(const void* value_ptr, size_t value_size) const {
    if (value_ptr == nullptr || value_size == 0)
      return false;
    if (!isValid())
      return false;
  
    // Send report attributes command
    esp_zb_zcl_report_attr_cmd_t report_attr_cmd;
    memset(&report_attr_cmd, 0, sizeof(report_attr_cmd));
    report_attr_cmd.zcl_basic_cmd.src_endpoint = _endpoint;
    report_attr_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report_attr_cmd.clusterID = _cluster_id;
    report_attr_cmd.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
    report_attr_cmd.manuf_code = _manuf_code;
    report_attr_cmd.attributeID = _attr_id;

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_err_t err = esp_zb_zcl_report_attr_cmd_req(&report_attr_cmd);
    esp_zb_lock_release();
    if (err != ESP_OK) {
      const char * err_name = esp_err_to_name(err);
      const char * access_name = zbAccessName();
      logEntry("WARNING: Failed to report attribute %s. Error: 0x%x: (%s). Attribute access: %s (0x%02x)", toString().c_str(), err, err_name, access_name, _access_id);
      return false;
    }
    return true;
  }

};
