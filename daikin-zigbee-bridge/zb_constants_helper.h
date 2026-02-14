#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "zcl/esp_zigbee_zcl_core.h"
#include "zcl/esp_zigbee_zcl_power_config.h"
#include "zdo/esp_zigbee_zdo_common.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char* UNKNOWN_HA_STANDARD_DEVICE_ID = "Unknown Device";
static const char* UNKNOWN_CLUSTER_ID = "Unknown Cluster";
static const char* UNKNOWN_ZCL_STATUS_ID = "Unknown ZCL Status";
static const char* UNKNOWN_ZDP_STATUS_ID = "Unknown ZDP Status";
static const char* UNKNOWN_ATTR_TYPE = "Unknown Attribute Type";
static const char* UNKNOWN_ATTR_ACCESS = "Unknown Access Type";
static const char* UNKNOWN_BASIC_CLUSTER_ATTR = "Unknown Basic Cluster Attribute";
static const char* UNKNOWN_POWER_CONFIG_CLUSTER_ATTR = "Unknown Power Config Cluster Attribute";
static const char* UNKNOWN_DEVICE_TEMPERATURE_CLUSTER_ATTR = "Unknown Device Temperature Cluster Attribute";
static const char* UNKNOWN_IDENTIFY_CLUSTER_ATTR = "Unknown Identify Cluster Attribute";
static const char* UNKNOWN_THERMOSTAT_CLUSTER_ATTR = "Unknown Thermostat Cluster Attribute";
static const char* UNKNOWN_THERMOSTAT_UI_CLUSTER_ATTR = "Unknown Thermostat Cluster Attribute";
static const char* UNKNOWN_METERING_CLUSTER_ATTR = "Unknown Metering Cluster Attribute";
static const char* UNKNOWN_CALLBACK_ID = "Unknown Callback";
static const char* UNKNOWN_SIGNAL_ID = "Unknown Signal";
static const char* UNKNOWN_SMART_CLUSTER_ATTR = "Unknown Smart Cluster Attribute";

static const char* zb_constants_ha_standard_device_id_to_string(esp_zb_ha_standard_devices_t value) {
  switch (value) {
    case ESP_ZB_HA_ON_OFF_SWITCH_DEVICE_ID:              return "General On/Off Switch";
    case ESP_ZB_HA_LEVEL_CONTROL_SWITCH_DEVICE_ID:       return "Level Control Switch";
    case ESP_ZB_HA_ON_OFF_OUTPUT_DEVICE_ID:              return "General On/Off Output";
    case ESP_ZB_HA_LEVEL_CONTROLLABLE_OUTPUT_DEVICE_ID:  return "Level Controllable Output";
    case ESP_ZB_HA_SCENE_SELECTOR_DEVICE_ID:             return "Scene Selector";
    case ESP_ZB_HA_CONFIGURATION_TOOL_DEVICE_ID:         return "Configuration Tool";
    case ESP_ZB_HA_REMOTE_CONTROL_DEVICE_ID:             return "Remote Control";
    case ESP_ZB_HA_COMBINED_INTERFACE_DEVICE_ID:         return "Combined Interface";
    case ESP_ZB_HA_RANGE_EXTENDER_DEVICE_ID:             return "Range Extender";
    case ESP_ZB_HA_MAINS_POWER_OUTLET_DEVICE_ID:         return "Mains Power Outlet";
    case ESP_ZB_HA_DOOR_LOCK_DEVICE_ID:                  return "Door Lock Client";
    case ESP_ZB_HA_DOOR_LOCK_CONTROLLER_DEVICE_ID:       return "Door Lock Controller";
    case ESP_ZB_HA_SIMPLE_SENSOR_DEVICE_ID:              return "Simple Sensor";
    case ESP_ZB_HA_CONSUMPTION_AWARENESS_DEVICE_ID:      return "Consumption Awareness";
    case ESP_ZB_HA_HOME_GATEWAY_DEVICE_ID:               return "Home Gateway";
    case ESP_ZB_HA_SMART_PLUG_DEVICE_ID:                 return "Smart Plug";
    case ESP_ZB_HA_WHITE_GOODS_DEVICE_ID:                return "White Goods";
    case ESP_ZB_HA_METER_INTERFACE_DEVICE_ID:            return "Meter Interface";
    case ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID:               return "On/Off Light";
    case ESP_ZB_HA_DIMMABLE_LIGHT_DEVICE_ID:             return "Dimmable Light";
    case ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID:       return "Color Dimmable Light";
    case ESP_ZB_HA_DIMMER_SWITCH_DEVICE_ID:              return "Dimmer Switch";
    case ESP_ZB_HA_COLOR_DIMMER_SWITCH_DEVICE_ID:        return "Color Dimmer Switch";
    case ESP_ZB_HA_SHADE_DEVICE_ID:                      return "Shade";
    case ESP_ZB_HA_SHADE_CONTROLLER_DEVICE_ID:           return "Shade Controller";
    case ESP_ZB_HA_WINDOW_COVERING_DEVICE_ID:            return "Window Covering Client";
    case ESP_ZB_HA_WINDOW_COVERING_CONTROLLER_DEVICE_ID: return "Window Covering Controller";
    case ESP_ZB_HA_HEATING_COOLING_UNIT_DEVICE_ID:       return "Heating/Cooling Unit";
    case ESP_ZB_HA_THERMOSTAT_DEVICE_ID:                 return "Thermostat";
    case ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID:         return "Temperature Sensor";
    case ESP_ZB_HA_IAS_CONTROL_INDICATING_EQUIPMENT_ID:  return "IAS Control and Indicating Equipment";
    case ESP_ZB_HA_IAS_ANCILLARY_CONTROL_EQUIPMENT_ID:   return "IAS Ancillary Control Equipment";
    case ESP_ZB_HA_IAS_ZONE_ID:                          return "IAS Zone";
    case ESP_ZB_HA_IAS_WARNING_DEVICE_ID:                return "IAS Warning";
    case ESP_ZB_HA_TEST_DEVICE_ID:                       return "Custom HA Test";
    case ESP_ZB_HA_CUSTOM_TUNNEL_DEVICE_ID:              return "Custom Tunnel";
    case ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID:                return "Custom Attributes";
    default:                                             return UNKNOWN_HA_STANDARD_DEVICE_ID;
  }
}

static const char* zb_constants_cluster_id_to_string(esp_zb_zcl_cluster_id_t value) {
  switch (value) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:                       return "Basic";
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:                return "Power Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:          return "Device Temperature Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:                    return "Identify";
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:                      return "Groups";
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:                      return "Scenes";
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:                      return "On/Off";
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG:        return "On/Off Switch Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:               return "Level Control";
    case ESP_ZB_ZCL_CLUSTER_ID_ALARMS:                      return "Alarms";
    case ESP_ZB_ZCL_CLUSTER_ID_TIME:                        return "Time";
    case ESP_ZB_ZCL_CLUSTER_ID_RSSI_LOCATION:               return "RSSI Location";
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT:                return "Analog Input";
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT:               return "Analog Output";
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE:                return "Analog Value";
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT:                return "Binary Input";
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_OUTPUT:               return "Binary Output";
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_VALUE:                return "Binary Value";
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_INPUT:                 return "Multistate Input";
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_OUTPUT:                return "Multistate Output";
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE:                 return "Multistate Value";
    case ESP_ZB_ZCL_CLUSTER_ID_COMMISSIONING:               return "Commissioning";
    case ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE:                 return "OTA Upgrade";
    case ESP_ZB_ZCL_CLUSTER_ID_POLL_CONTROL:                return "Poll Control";
    case ESP_ZB_ZCL_CLUSTER_ID_GREEN_POWER:                 return "Green Power";
    case ESP_ZB_ZCL_CLUSTER_ID_KEEP_ALIVE:                  return "Keep Alive";
    case ESP_ZB_ZCL_CLUSTER_ID_SHADE_CONFIG:                return "Shade Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_DOOR_LOCK:                   return "Door Lock";
    case ESP_ZB_ZCL_CLUSTER_ID_WINDOW_COVERING:             return "Window Covering";
    case ESP_ZB_ZCL_CLUSTER_ID_PUMP_CONFIG_CONTROL:         return "Pump Configuration and Control";
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT:                  return "Thermostat";
    case ESP_ZB_ZCL_CLUSTER_ID_FAN_CONTROL:                 return "Fan Control";
    case ESP_ZB_ZCL_CLUSTER_ID_DEHUMIDIFICATION_CONTROL /*ESP_ZB_ZCL_CLUSTER_ID_DEHUMID_CONTROL*/:             return "Dehumidification Control";
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG:        return "Thermostat UI Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL:               return "Color Control";
    case ESP_ZB_ZCL_CLUSTER_ID_BALLAST_CONFIG:              return "Ballast Configuration";
    case ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT:     return "Illuminance Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:            return "Temperature Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT:        return "Pressure Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:    return "Relative Humidity Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING:           return "Occupancy Sensing";
    case ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT:  return "Carbon Dioxide Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_PM2_5_MEASUREMENT:           return "PM2.5 Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE:                    return "IAS Zone";
    case ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT:      return "Electrical Measurement";
    case ESP_ZB_ZCL_CLUSTER_ID_METERING:                    return "Metering";
    default:                                                return UNKNOWN_CLUSTER_ID;
  }
}

static const char* zb_constants_zcl_status_to_string(esp_zb_zcl_status_t value) {
  switch (value) {
    case ESP_ZB_ZCL_STATUS_SUCCESS:               return "ZCL Success";
    case ESP_ZB_ZCL_STATUS_FAIL:                  return "ZCL Fail";
    case ESP_ZB_ZCL_STATUS_NOT_AUTHORIZED:        return "Not Authorized";
    case ESP_ZB_ZCL_STATUS_MALFORMED_CMD:         return "Malformed Command";
    case ESP_ZB_ZCL_STATUS_UNSUP_CLUST_CMD:       return "Unsupported Cluster Command";
    case ESP_ZB_ZCL_STATUS_UNSUP_GEN_CMD:         return "Unsupported General Command";
    case ESP_ZB_ZCL_STATUS_UNSUP_MANUF_CLUST_CMD: return "Unsupported Manufacturer-Specific Cluster Command";
    case ESP_ZB_ZCL_STATUS_UNSUP_MANUF_GEN_CMD:   return "Unsupported Manufacturer-Specific General Command";
    case ESP_ZB_ZCL_STATUS_INVALID_FIELD:         return "Invalid Field";
    case ESP_ZB_ZCL_STATUS_UNSUP_ATTRIB:          return "Unsupported Attribute";
    case ESP_ZB_ZCL_STATUS_INVALID_VALUE:         return "Invalid Value";
    case ESP_ZB_ZCL_STATUS_READ_ONLY:             return "Read Only";
    case ESP_ZB_ZCL_STATUS_INSUFF_SPACE:          return "Insufficient Space";
    case ESP_ZB_ZCL_STATUS_DUPE_EXISTS:           return "Duplicate Exists";
    case ESP_ZB_ZCL_STATUS_NOT_FOUND:             return "Not Found";
    case ESP_ZB_ZCL_STATUS_UNREPORTABLE_ATTRIB:   return "Unreportable Attribute";
    case ESP_ZB_ZCL_STATUS_INVALID_TYPE:          return "Invalid Type";
    case ESP_ZB_ZCL_STATUS_WRITE_ONLY:            return "Write Only";
    case ESP_ZB_ZCL_STATUS_INCONSISTENT:          return "Inconsistent Values";
    case ESP_ZB_ZCL_STATUS_ACTION_DENIED:         return "Action Denied";
    case ESP_ZB_ZCL_STATUS_TIMEOUT:               return "Timeout";
    case ESP_ZB_ZCL_STATUS_ABORT:                 return "Abort";
    case ESP_ZB_ZCL_STATUS_INVALID_IMAGE:         return "Invalid OTA Image";
    case ESP_ZB_ZCL_STATUS_WAIT_FOR_DATA:         return "Wait for Data";
    case ESP_ZB_ZCL_STATUS_NO_IMAGE_AVAILABLE:    return "No Image Available";
    case ESP_ZB_ZCL_STATUS_REQUIRE_MORE_IMAGE:    return "Require More Image";
    case ESP_ZB_ZCL_STATUS_NOTIFICATION_PENDING:  return "Notification Pending";
    case ESP_ZB_ZCL_STATUS_HW_FAIL:               return "Hardware Failure";
    case ESP_ZB_ZCL_STATUS_SW_FAIL:               return "Software Failure";
    case ESP_ZB_ZCL_STATUS_CALIB_ERR:             return "Calibration Error";
    case ESP_ZB_ZCL_STATUS_UNSUP_CLUST:           return "Unsupported";
    case ESP_ZB_ZCL_STATUS_LIMIT_REACHED:         return "Limit Reached";
    default:                                      return UNKNOWN_ZCL_STATUS_ID;
  }
}

static const char* zb_constants_zcl_attr_type_to_string(esp_zb_zcl_attr_type_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_TYPE_NULL:              return "Null Data Type";
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:              return "8-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:             return "16-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_24BIT:             return "24-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:             return "32-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_40BIT:             return "40-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_48BIT:             return "48-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_56BIT:             return "56-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_64BIT:             return "64-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_BOOL:              return "Boolean";
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:           return "8-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:          return "16-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP:          return "24-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:          return "32-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP:          return "40-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP:          return "48-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP:          return "56-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP:          return "64-bit Bitmap";
    case ESP_ZB_ZCL_ATTR_TYPE_U8:                return "Unsigned 8-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U16:               return "Unsigned 16-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U24:               return "Unsigned 24-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U32:               return "Unsigned 32-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U40:               return "Unsigned 40-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U48:               return "Unsigned 48-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U56:               return "Unsigned 56-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_U64:               return "Unsigned 64-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S8:                return "Signed 8-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S16:               return "Signed 16-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S24:               return "Signed 24-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S32:               return "Signed 32-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S40:               return "Signed 40-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S48:               return "Signed 48-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S56:               return "Signed 56-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_S64:               return "Signed 64-bit Value";
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:         return "8-bit Enumeration";
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:        return "16-bit Enumeration";
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI:              return "2-byte Floating Point";
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE:            return "4-byte Floating Point";
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:            return "8-byte Floating Point";
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:      return "Octet String";
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:       return "Character String";
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING: return "Long Octet String";
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:  return "Long Character String";
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:             return "Array (8-bit Type)";
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ARRAY:       return "Array (16-bit Type)";
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT_ARRAY:       return "Array (32-bit Type)";
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:         return "Structure";
    case ESP_ZB_ZCL_ATTR_TYPE_SET:               return "Set Collection";
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:               return "Bag Collection";
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:       return "Time of Day";
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:              return "Date";
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:          return "UTC Time";
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:        return "Cluster ID";
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:      return "Attribute ID";
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:        return "BACnet OID";
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:         return "IEEE Address";
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:       return "128-bit Security Key";
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:           return "Invalid Data Type";
    default:                                     return UNKNOWN_ATTR_TYPE;
  }
}

static const char* zb_constants_zcl_attr_access_to_string(esp_zb_zcl_attr_access_t value) {
  switch (value) {
    case 0:                                   return "Unset"; // UNDOCUMENTED. Assumption

    // Base values
    case ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:    return "Read Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:   return "Write Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:   return "Read/Write";

    // Flags
    case ESP_ZB_ZCL_ATTR_ACCESS_REPORTING:    return "Reporting Allowed";
    case ESP_ZB_ZCL_ATTR_ACCESS_SINGLETON:    return "Singleton";
    case ESP_ZB_ZCL_ATTR_ACCESS_SCENE:        return "Scene";
    case ESP_ZB_ZCL_ATTR_MANUF_SPEC:          return "Manufacturer Specific";
    case ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL:     return "Internal Only";

    // Combinations:
    case ESP_ZB_ZCL_ATTR_ACCESS_REPORTING + ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:   return "Reporting, Read Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_REPORTING + ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:  return "Reporting, Write Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_REPORTING + ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:  return "Reporting, Read/Write";

    case ESP_ZB_ZCL_ATTR_ACCESS_SINGLETON + ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:   return "Singleton, Read Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_SINGLETON + ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:  return "Singleton, Write Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_SINGLETON + ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:  return "Singleton, Read/Write";

    case ESP_ZB_ZCL_ATTR_ACCESS_SCENE + ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:   return "Scene, Read Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_SCENE + ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:  return "Scene, Write Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_SCENE + ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:  return "Scene, Read/Write";

    case ESP_ZB_ZCL_ATTR_MANUF_SPEC + ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:   return "Manufacturer Specific, Read Only";
    case ESP_ZB_ZCL_ATTR_MANUF_SPEC + ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:  return "Manufacturer Specific, Write Only";
    case ESP_ZB_ZCL_ATTR_MANUF_SPEC + ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:  return "Manufacturer Specific, Read/Write";

    case ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL + ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY:    return "Internal Only, Read Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL + ESP_ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:   return "Internal Only, Write Only";
    case ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL + ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE:   return "Internal Only, Read/Write";

    default: return UNKNOWN_ATTR_ACCESS;
  }
}

static const char* zb_constants_zcl_basic_cluster_attr_to_string(esp_zb_zcl_basic_attr_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID:              return "ZCL Version";
    case ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID:      return "Application Version";
    case ESP_ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID:            return "Stack Version";
    case ESP_ZB_ZCL_ATTR_BASIC_HW_VERSION_ID:               return "Hardware Version";
    case ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID:        return "Manufacturer Name";
    case ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID:         return "Model Identifier";
    case ESP_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID:                return "Date Code";
    case ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID:             return "Power Source";
    case ESP_ZB_ZCL_ATTR_BASIC_GENERIC_DEVICE_CLASS_ID:     return "Generic Device Class";
    case ESP_ZB_ZCL_ATTR_BASIC_GENERIC_DEVICE_TYPE_ID:      return "Generic Device Type";
    case ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_CODE_ID:             return "Product Code";
    case ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_URL_ID:              return "Product URL";
    case ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_VERSION_DETAILS_ID: return "Manufacturer Version Details";
    case ESP_ZB_ZCL_ATTR_BASIC_SERIAL_NUMBER_ID:            return "Serial Number";
    case ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_LABEL_ID:            return "Product Label";
    case ESP_ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID:     return "Location Description";
    case ESP_ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID:     return "Physical Environment";
    case ESP_ZB_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID:           return "Device Enabled";
    case ESP_ZB_ZCL_ATTR_BASIC_ALARM_MASK_ID:               return "Alarm Mask";
    case ESP_ZB_ZCL_ATTR_BASIC_DISABLE_LOCAL_CONFIG_ID:     return "Disable Local Config";
    case ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID:                 return "SW Build";
    default:                                                return UNKNOWN_BASIC_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_power_config_cluster_attr_to_string(esp_zb_zcl_power_config_attr_t value) {
  switch (value) {
    /* Mains Attributes */
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_ID:            return "Mains Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_FREQUENCY_ID:          return "Mains Frequency";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_ALARM_MASK_ID:         return "Mains Alarm Mask";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_MIN_THRESHOLD: return "Mains Voltage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_MAX_THRESHOLD: return "Mains Voltage Max Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_DWELL_TRIP_POINT:      return "Mains Dwell Trip Point";

    /* Battery 1 Attributes */
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID:              return "Battery Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID: return "Battery Percentage Remaining";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_MANUFACTURER_ID:         return "Battery Manufacturer";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_SIZE_ID:                 return "Battery Size";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_A_HR_RATING_ID:          return "Battery AH Rating";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_QUANTITY_ID:             return "Battery Quantity";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID:        return "Battery Rated Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID:           return "Battery Alarm Mask";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_MIN_THRESHOLD_ID:return "Battery Voltage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_THRESHOLD1_ID:   return "Battery Voltage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_THRESHOLD2_ID:   return "Battery Voltage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_THRESHOLD3_ID:   return "Battery Voltage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_MIN_THRESHOLD_ID: return "Battery Percentage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_THRESHOLD1_ID:    return "Battery Percentage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_THRESHOLD2_ID:    return "Battery Percentage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_THRESHOLD3_ID:    return "Battery Percentage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_STATE_ID:          return "Battery Alarm State";

    /* Battery 2 Attributes */
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_VOLTAGE_ID:              return "Battery 2 Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_PERCENTAGE_REMAINING_ID: return "Battery 2 Percentage Remaining";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_MANUFACTURER_ID:         return "Battery 2 Manufacturer";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_SIZE_ID:                 return "Battery 2 Size";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_A_HR_RATING_ID:          return "Battery 2 AH Rating";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_QUANTITY_ID:             return "Battery 2 Quantity";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_RATED_VOLTAGE_ID:        return "Battery 2 Rated Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_ALARM_MASK_ID:           return "Battery 2 Alarm Mask";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_VOLTAGE_MIN_THRESHOLD_ID:return "Battery 2 Voltage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_VOLTAGE_THRESHOLD1_ID:   return "Battery 2 Voltage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_VOLTAGE_THRESHOLD2_ID:   return "Battery 2 Voltage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_VOLTAGE_THRESHOLD3_ID:   return "Battery 2 Voltage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_PERCENTAGE_MIN_THRESHOLD_ID: return "Battery 2 Percentage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_PERCENTAGE_THRESHOLD1_ID:    return "Battery 2 Percentage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_PERCENTAGE_THRESHOLD2_ID:    return "Battery 2 Percentage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_PERCENTAGE_THRESHOLD3_ID:    return "Battery 2 Percentage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY2_ALARM_STATE_ID:          return "Battery 2 Alarm State";

    /* Battery 3 Attributes */
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_VOLTAGE_ID:              return "Battery 3 Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_PERCENTAGE_REMAINING_ID: return "Battery 3 Percentage Remaining";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_MANUFACTURER_ID:         return "Battery 3 Manufacturer";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_SIZE_ID:                 return "Battery 3 Size";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_A_HR_RATING_ID:          return "Battery 3 AH Rating";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_QUANTITY_ID:             return "Battery 3 Quantity";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_RATED_VOLTAGE_ID:        return "Battery 3 Rated Voltage";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_ALARM_MASK_ID:           return "Battery 3 Alarm Mask";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_VOLTAGE_MIN_THRESHOLD_ID:return "Battery 3 Voltage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_VOLTAGE_THRESHOLD1_ID:   return "Battery 3 Voltage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_VOLTAGE_THRESHOLD2_ID:   return "Battery 3 Voltage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_VOLTAGE_THRESHOLD3_ID:   return "Battery 3 Voltage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_PERCENTAGE_MIN_THRESHOLD_ID: return "Battery 3 Percentage Min Threshold";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_PERCENTAGE_THRESHOLD1_ID:    return "Battery 3 Percentage Threshold 1";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_PERCENTAGE_THRESHOLD2_ID:    return "Battery 3 Percentage Threshold 2";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_PERCENTAGE_THRESHOLD3_ID:    return "Battery 3 Percentage Threshold 3";
    case ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY3_ALARM_STATE_ID:          return "Battery 3 Alarm State";

    default: return UNKNOWN_POWER_CONFIG_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_device_temp_config_cluster_attr_to_string(esp_zb_zcl_device_temp_config_attr_e value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_CURRENT_TEMP_ID:                return "Current Temperature";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_MIN_TEMP_EXPERIENCED_ID:        return "Minimum Temperature Experienced";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_MAX_TEMP_EXPERIENCED_ID:        return "Maximum Temperature Experienced";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_OVER_TEMP_TOTAL_DWELL_ID:       return "Over Temperature Total Dwell";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_DEVICE_TEMP_ALARM_MASK_ID:      return "Device Temperature Alarm Mask";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_LOW_TEMP_THRESHOLD_ID:          return "Low Temperature Threshold";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_HIGH_TEMP_THRESHOLD_ID:         return "High Temperature Threshold";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_LOW_TEMP_DWELL_TRIP_POINT_ID:   return "Low Temperature Dwell Trip Point";
    case ESP_ZB_ZCL_ATTR_DEVICE_TEMP_CONFIG_HIGH_TEMP_DWELL_TRIP_POINT_ID:  return "High Temperature Dwell Trip Point";
    default: return UNKNOWN_DEVICE_TEMPERATURE_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_identify_cluster_attr_to_string(esp_zb_zcl_identify_attr_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID:   return "Identify Time";
    default: return UNKNOWN_IDENTIFY_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_thermostat_cluster_attr_to_string(esp_zb_zcl_thermostat_attr_t value) {
  switch (value) {
    /* Core Temperature & Control Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID:             return "Local Temperature";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID:           return "Outdoor Temperature";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID:                     return "Occupancy";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID:   return "Abs Min Heat Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID:   return "Abs Max Heat Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID:   return "Abs Min Cool Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID:   return "Abs Max Cool Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_COOLING_DEMAND_ID:             return "PI Cooling Demand";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID:             return "PI Heating Demand";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION_ID:return "HVAC System Type Configuration";

    /* Setpoint Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_ID: return "Local Temperature Calibration";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID:     return "Occupied Cooling Setpoint";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID:     return "Occupied Heating Setpoint";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT_ID:   return "Unoccupied Cooling Setpoint";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT_ID:   return "Unoccupied Heating Setpoint";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID:       return "Min Heat Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID:       return "Max Heat Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID:       return "Min Cool Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID:       return "Max Cool Setpoint Limit";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_SETPOINT_DEAD_BAND_ID:        return "Min Setpoint Dead Band";

    /* System Control Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_REMOTE_SENSING_ID:                return "Remote Sensing";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID: return "Control Sequence Of Operation";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID:                   return "System Mode";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_ALARM_MASK_ID:                    return "Alarm Mask";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_RUNNING_MODE_ID:                  return "Running Mode";

    /* Schedule & Programming Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_START_OF_WEEK_ID:                         return "Start of Week";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS_ID:          return "Number of Weekly Transitions";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS_ID:           return "Number of Daily Transitions";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_ID:             return "Temperature Setpoint Hold";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION_ID:    return "Temperature Setpoint Hold Duration";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_ID: return "Thermostat Programming Operation Mode";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID:              return "Thermostat Running State";

    /* Event & Logging Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_SOURCE_ID:                return "Setpoint Change Source";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_AMOUNT_ID:                return "Setpoint Change Amount";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP_ID:      return "Setpoint Change Source Timestamp";

    /* Setback & Emergency Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_ID:              return "Occupied Setback";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_MIN_ID:          return "Occupied Setback Min";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_SETBACK_MAX_ID:          return "Occupied Setback Max";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_ID:            return "Unoccupied Setback";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_MIN_ID:        return "Unoccupied Setback Min";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UNOCCUPIED_SETBACK_MAX_ID:        return "Unoccupied Setback Max";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_EMERGENCY_HEAT_DELTA_ID:          return "Emergency Heat Delta";

    /* Mini Split AC Specific Attributes */
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_TYPE_ID:                       return "AC Type";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_CAPACITY_ID:                   return "AC Capacity";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_REFRIGERANT_TYPE_ID:           return "AC Refrigerant Type";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_COMPRESSOR_TYPE_ID:            return "AC Compressor Type";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_ERROR_CODE_ID:                 return "AC Error Code";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_LOUVER_POSITION_ID:            return "AC Louver Position";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_COIL_TEMPERATURE_ID:           return "AC Coil Temperature";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_AC_CAPACITY_FORMAT_ID:            return "AC Capacity Format";

    default: return UNKNOWN_THERMOSTAT_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_thermostat_ui_cluster_attr_to_string(esp_zb_zcl_thermostat_ui_config_attr_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID:        return "Temperature Display Mode";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID:                  return "Keypad Lockout";
    case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_SCHEDULE_PROGRAMMING_VISIBILITY_ID: return "Schedule Programming Visibility";
    default: return UNKNOWN_THERMOSTAT_UI_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zcl_metering_cluster_attr_to_string(esp_zb_zcl_metering_attr_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_METERING_INSTANTANEOUS_DEMAND_ID:            return "Instantaneous Demand";
    case ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID:     return "Current Summation Delivered";
    case ESP_ZB_ZCL_ATTR_METERING_UNIT_OF_MEASURE_ID:                 return "Unit of Measure";
    case ESP_ZB_ZCL_ATTR_METERING_MULTIPLIER_ID:                      return "Multiplier";
    case ESP_ZB_ZCL_ATTR_METERING_DIVISOR_ID:                         return "Divisor";
    default: return UNKNOWN_METERING_CLUSTER_ATTR;
  }
}

static const char* zb_constants_zdp_status_to_string(esp_zb_zdp_status_t status) {
  switch (status) {
    case ESP_ZB_ZDP_STATUS_SUCCESS:            return "Success";
    case ESP_ZB_ZDP_STATUS_INV_REQUESTTYPE:    return "Invalid Request Type";
    case ESP_ZB_ZDP_STATUS_DEVICE_NOT_FOUND:   return "Device Not Found";
    case ESP_ZB_ZDP_STATUS_INVALID_EP:         return "Invalid Endpoint";
    case ESP_ZB_ZDP_STATUS_NOT_ACTIVE:         return "Not Active";
    case ESP_ZB_ZDP_STATUS_NOT_SUPPORTED:      return "Not Supported";
    case ESP_ZB_ZDP_STATUS_TIMEOUT:            return "Timeout";
    case ESP_ZB_ZDP_STATUS_NO_MATCH:           return "No Match";
    case ESP_ZB_ZDP_STATUS_NO_ENTRY:           return "No Entry";
    case ESP_ZB_ZDP_STATUS_NO_DESCRIPTOR:      return "No Descriptor";
    case ESP_ZB_ZDP_STATUS_INSUFFICIENT_SPACE: return "Insufficient Space";
    case ESP_ZB_ZDP_STATUS_NOT_PERMITTED:      return "Not Permitted";
    case ESP_ZB_ZDP_STATUS_TABLE_FULL:         return "Table Full";
    case ESP_ZB_ZDP_STATUS_NOT_AUTHORIZED:     return "Not Authorized";
    case ESP_ZB_ZDP_STATUS_BINDING_TABLE_FULL: return "Binding Table Full";
    case ESP_ZB_ZDP_STATUS_INVALID_INDEX:      return "Invalid Index";
    default: return UNKNOWN_ZDP_STATUS_ID;
  }
}

static const char* zb_constants_core_action_callback_id_to_string(esp_zb_core_action_callback_id_t value) {
  switch (value) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:                      return "Set Attribute Value";
    case ESP_ZB_CORE_SCENES_STORE_SCENE_CB_ID:                  return "Scenes Store Scene";
    case ESP_ZB_CORE_SCENES_RECALL_SCENE_CB_ID:                 return "Scenes Recall Scene";
    case ESP_ZB_CORE_IAS_ZONE_ENROLL_RESPONSE_VALUE_CB_ID:      return "IAS Zone Enroll Response";
    case ESP_ZB_CORE_OTA_UPGRADE_VALUE_CB_ID:                   return "OTA Upgrade Value";
    case ESP_ZB_CORE_OTA_UPGRADE_SRV_STATUS_CB_ID:              return "OTA Server Status";
    case ESP_ZB_CORE_OTA_UPGRADE_SRV_QUERY_IMAGE_CB_ID:         return "OTA Server Query Image";
    case ESP_ZB_CORE_THERMOSTAT_VALUE_CB_ID:                    return "Thermostat Value";
    case ESP_ZB_CORE_METERING_GET_PROFILE_CB_ID:                return "Metering Get Profile";
    case ESP_ZB_CORE_METERING_GET_PROFILE_RESP_CB_ID:           return "Metering Get Profile Response";
    case ESP_ZB_CORE_METERING_REQ_FAST_POLL_MODE_CB_ID:         return "Metering Request Fast Poll Mode";
    case ESP_ZB_CORE_METERING_REQ_FAST_POLL_MODE_RESP_CB_ID:    return "Metering Request Fast Poll Mode Response";
    case ESP_ZB_CORE_METERING_GET_SNAPSHOT_CB_ID:               return "Metering Get Snapshot";
    case ESP_ZB_CORE_METERING_PUBLISH_SNAPSHOT_CB_ID:           return "Metering Publish Snapshot";
    case ESP_ZB_CORE_METERING_GET_SAMPLED_DATA_CB_ID:           return "Metering Get Sampled Data";
    case ESP_ZB_CORE_METERING_GET_SAMPLED_DATA_RESP_CB_ID:      return "Metering Get Sampled Data Response";
    case ESP_ZB_CORE_DOOR_LOCK_LOCK_DOOR_CB_ID:                 return "Door Lock Request";
    case ESP_ZB_CORE_DOOR_LOCK_LOCK_DOOR_RESP_CB_ID:            return "Door Lock Response";
    case ESP_ZB_CORE_IDENTIFY_EFFECT_CB_ID:                     return "Identify Effect";
    case ESP_ZB_CORE_BASIC_RESET_TO_FACTORY_RESET_CB_ID:        return "Basic Reset to Factory Default";
    case ESP_ZB_CORE_PRICE_GET_CURRENT_PRICE_CB_ID:             return "Price Get Current Price";
    case ESP_ZB_CORE_PRICE_GET_SCHEDULED_PRICES_CB_ID:          return "Price Get Scheduled Prices";
    case ESP_ZB_CORE_PRICE_GET_TIER_LABELS_CB_ID:               return "Price Get Tier Labels";
    case ESP_ZB_CORE_PRICE_PUBLISH_PRICE_CB_ID:                 return "Price Publish Price";
    case ESP_ZB_CORE_PRICE_PUBLISH_TIER_LABELS_CB_ID:           return "Price Publish Tier Labels";
    case ESP_ZB_CORE_PRICE_PRICE_ACK_CB_ID:                     return "Price Acknowledgement";
    case ESP_ZB_CORE_COMM_RESTART_DEVICE_CB_ID:                 return "Commissioning Restart Device";
    case ESP_ZB_CORE_COMM_OPERATE_STARTUP_PARAMS_CB_ID:         return "Commissioning Operate Startup Parameters";
    case ESP_ZB_CORE_COMM_COMMAND_RESP_CB_ID:                   return "Commissioning Command Response";
    case ESP_ZB_CORE_IAS_WD_START_WARNING_CB_ID:                return "IAS WD Start Warning";
    case ESP_ZB_CORE_IAS_WD_SQUAWK_CB_ID:                       return "IAS WD Squawk";
    case ESP_ZB_CORE_IAS_ACE_ARM_CB_ID:                         return "IAS ACE Arm";
    case ESP_ZB_CORE_IAS_ACE_BYPASS_CB_ID:                      return "IAS ACE Bypass";
    case ESP_ZB_CORE_IAS_ACE_EMERGENCY_CB_ID:                   return "IAS ACE Emergency";
    case ESP_ZB_CORE_IAS_ACE_FIRE_CB_ID:                        return "IAS ACE Fire";
    case ESP_ZB_CORE_IAS_ACE_PANIC_CB_ID:                       return "IAS ACE Panic";
    case ESP_ZB_CORE_IAS_ACE_GET_PANEL_STATUS_CB_ID:            return "IAS ACE Get Panel Status";
    case ESP_ZB_CORE_IAS_ACE_GET_BYPASSED_ZONE_LIST_CB_ID:      return "IAS ACE Get Bypassed Zone List";
    case ESP_ZB_CORE_IAS_ACE_GET_ZONE_STATUS_CB_ID:             return "IAS ACE Get Zone Status";
    case ESP_ZB_CORE_IAS_ACE_ARM_RESP_CB_ID:                    return "IAS ACE Arm Response";
    case ESP_ZB_CORE_IAS_ACE_GET_ZONE_ID_MAP_RESP_CB_ID:        return "IAS ACE Get Zone ID Map Response";
    case ESP_ZB_CORE_IAS_ACE_GET_ZONE_INFO_RESP_CB_ID:          return "IAS ACE Get Zone Info Response";
    case ESP_ZB_CORE_IAS_ACE_ZONE_STATUS_CHANGED_CB_ID:         return "IAS ACE Zone Status Changed";
    case ESP_ZB_CORE_IAS_ACE_PANEL_STATUS_CHANGED_CB_ID:        return "IAS ACE Panel Status Changed";
    case ESP_ZB_CORE_IAS_ACE_GET_PANEL_STATUS_RESP_CB_ID:       return "IAS ACE Get Panel Status Response";
    case ESP_ZB_CORE_IAS_ACE_SET_BYPASSED_ZONE_LIST_CB_ID:      return "IAS ACE Set Bypassed Zone List";
    case ESP_ZB_CORE_IAS_ACE_BYPASS_RESP_CB_ID:                 return "IAS ACE Bypass Response";
    case ESP_ZB_CORE_IAS_ACE_GET_ZONE_STATUS_RESP_CB_ID:        return "IAS ACE Get Zone Status Response";
    case ESP_ZB_CORE_WINDOW_COVERING_MOVEMENT_CB_ID:            return "Window Covering Movement";
    case ESP_ZB_CORE_OTA_UPGRADE_QUERY_IMAGE_RESP_CB_ID:        return "OTA Upgrade Query Image Response";
    case ESP_ZB_CORE_THERMOSTAT_WEEKLY_SCHEDULE_SET_CB_ID:      return "Thermostat Weekly Schedule Set";
    case ESP_ZB_CORE_DRLC_LOAD_CONTROL_EVENT_CB_ID:             return "DRLC Load Control Event";
    case ESP_ZB_CORE_DRLC_CANCEL_LOAD_CONTROL_EVENT_CB_ID:      return "DRLC Cancel Load Control Event";
    case ESP_ZB_CORE_DRLC_CANCEL_ALL_LOAD_CONTROL_EVENTS_CB_ID: return "DRLC Cancel All Load Control Events";
    case ESP_ZB_CORE_DRLC_REPORT_EVENT_STATUS_CB_ID:            return "DRLC Report Event Status";
    case ESP_ZB_CORE_DRLC_GET_SCHEDULED_EVENTS_CB_ID:           return "DRLC Get Scheduled Events";
    case ESP_ZB_CORE_POLL_CONTROL_CHECK_IN_REQ_CB_ID:           return "Poll Control Check-In Request";
    case ESP_ZB_CORE_ALARMS_RESET_ALARM_CB_ID:                  return "Alarms Reset Alarm";
    case ESP_ZB_CORE_ALARMS_RESET_ALL_ALARMS_CB_ID:             return "Alarms Reset All Alarms";
    case ESP_ZB_CORE_ALARMS_ALARM_CB_ID:                        return "Alarms Alarm";
    case ESP_ZB_CORE_ALARMS_GET_ALARM_RESP_CB_ID:               return "Alarms Get Alarm Response";
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:                  return "Read Attribute Response";
    case ESP_ZB_CORE_CMD_WRITE_ATTR_RESP_CB_ID:                 return "Write Attribute Response";
    case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:              return "Configure Report Response";
    case ESP_ZB_CORE_CMD_READ_REPORT_CFG_RESP_CB_ID:            return "Read Report Configuration Response";
    case ESP_ZB_CORE_CMD_DISC_ATTR_RESP_CB_ID:                  return "Discover Attributes Response";
    case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:                    return "Default Response";
    case ESP_ZB_CORE_CMD_OPERATE_GROUP_RESP_CB_ID:              return "Groups Operate Group Response";
    case ESP_ZB_CORE_CMD_VIEW_GROUP_RESP_CB_ID:                 return "Groups View Group Response";
    case ESP_ZB_CORE_CMD_GET_GROUP_MEMBERSHIP_RESP_CB_ID:       return "Groups Get Membership Response";
    case ESP_ZB_CORE_CMD_OPERATE_SCENE_RESP_CB_ID:              return "Scenes Operate Scene Response";
    case ESP_ZB_CORE_CMD_VIEW_SCENE_RESP_CB_ID:                 return "Scenes View Scene Response";
    case ESP_ZB_CORE_CMD_GET_SCENE_MEMBERSHIP_RESP_CB_ID:       return "Scenes Get Membership Response";
    case ESP_ZB_CORE_CMD_IAS_ZONE_ZONE_ENROLL_REQUEST_ID:       return "IAS Zone Enroll Request";
    case ESP_ZB_CORE_CMD_IAS_ZONE_ZONE_STATUS_CHANGE_NOT_ID:    return "IAS Zone Status Change Notification";
    case ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_REQ_CB_ID:              return "Custom Cluster Request";
    case ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_RESP_CB_ID:             return "Custom Cluster Response";
    case ESP_ZB_CORE_CMD_PRIVILEGE_COMMAND_REQ_CB_ID:           return "Privilege Command Request";
    case ESP_ZB_CORE_CMD_PRIVILEGE_COMMAND_RESP_CB_ID:          return "Privilege Command Response";
    case ESP_ZB_CORE_CMD_TOUCHLINK_GET_GROUP_ID_RESP_CB_ID:     return "Touchlink Get Group ID Response";
    case ESP_ZB_CORE_CMD_TOUCHLINK_GET_ENDPOINT_LIST_RESP_CB_ID:return "Touchlink Get Endpoint List Response";
    case ESP_ZB_CORE_CMD_THERMOSTAT_GET_WEEKLY_SCHEDULE_RESP_CB_ID: return "Thermostat Get Weekly Schedule Response";
    case ESP_ZB_CORE_CMD_GREEN_POWER_RECV_CB_ID:                return "Green Power Command Receive";
    case ESP_ZB_CORE_REPORT_ATTR_CB_ID:                         return "Attribute Report";
    default: return UNKNOWN_CALLBACK_ID;
  }
}

static const char* zb_constants_app_signal_type_to_string(esp_zb_app_signal_type_t signal) {
  switch (signal) {
    case ESP_ZB_ZDO_SIGNAL_DEFAULT_START:               return "ZDO Default Start";
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:                return "ZDO Skip Startup";
    case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE:                return "ZDO Device Announce";
    case ESP_ZB_ZDO_SIGNAL_LEAVE:                       return "ZDO Leave";
    case ESP_ZB_ZDO_SIGNAL_ERROR:                       return "ZDO Error";
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:          return "BDB Device First Start";
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:               return "BDB Device Reboot";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_NWK_STARTED:       return "BDB Touchlink Network Started";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_NWK_JOINED_ROUTER: return "BDB Touchlink Network Joined Router";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK:                   return "BDB Touchlink";
    case ESP_ZB_BDB_SIGNAL_STEERING:                    return "BDB Steering";
    case ESP_ZB_BDB_SIGNAL_FORMATION:                   return "BDB Formation";
    case ESP_ZB_BDB_SIGNAL_FINDING_AND_BINDING_TARGET_FINISHED:    return "BDB Finding & Binding Target Finished";
    case ESP_ZB_BDB_SIGNAL_FINDING_AND_BINDING_INITIATOR_FINISHED: return "BDB Finding & Binding Initiator Finished";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_TARGET:            return "BDB Touchlink Target";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_NWK:               return "BDB Touchlink Network";
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_TARGET_FINISHED:   return "BDB Touchlink Target Finished";
    case ESP_ZB_NWK_SIGNAL_DEVICE_ASSOCIATED:           return "NWK Device Associated";
    case ESP_ZB_ZDO_SIGNAL_LEAVE_INDICATION:            return "ZDO Leave Indication";
    case ESP_ZB_ZGP_SIGNAL_COMMISSIONING:               return "ZGP Commissioning";
    case ESP_ZB_COMMON_SIGNAL_CAN_SLEEP:                return "Common Can Sleep";
    case ESP_ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:     return "ZDO Production Config Ready";
    case ESP_ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT:        return "NWK No Active Links Left";
    case ESP_ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED:           return "ZDO Device Authorized";
    case ESP_ZB_ZDO_SIGNAL_DEVICE_UPDATE:               return "ZDO Device Update";
    case ESP_ZB_NWK_SIGNAL_PANID_CONFLICT_DETECTED:     return "NWK PAN ID Conflict Detected";
    case ESP_ZB_NLME_STATUS_INDICATION:                 return "NLME Status Indication";
    case ESP_ZB_BDB_SIGNAL_TC_REJOIN_DONE:              return "BDB TC Rejoin Done";
    case ESP_ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS:          return "NWK Permit Join Status";
    case ESP_ZB_BDB_SIGNAL_STEERING_CANCELLED:          return "BDB Steering Cancelled";
    case ESP_ZB_BDB_SIGNAL_FORMATION_CANCELLED:         return "BDB Formation Cancelled";
    case ESP_ZB_ZGP_SIGNAL_MODE_CHANGE:                 return "ZGP Mode Change";
    case ESP_ZB_ZDO_DEVICE_UNAVAILABLE:                 return "ZDO Device Unavailable";
    case ESP_ZB_ZGP_SIGNAL_APPROVE_COMMISSIONING:       return "ZGP Approve Commissioning";
    case ESP_ZB_SIGNAL_END:                             return "Signal End";
    default: return UNKNOWN_SIGNAL_ID;
  }
}

static const char* zb_constants_smart_cluster_attr_to_string(esp_zb_zcl_cluster_id_t cluster, int value) {
  switch (cluster) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:                       return zb_constants_zcl_basic_cluster_attr_to_string((esp_zb_zcl_basic_attr_t)value);
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:                return zb_constants_zcl_power_config_cluster_attr_to_string((esp_zb_zcl_power_config_attr_t)value);
    case ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:          return zb_constants_zcl_device_temp_config_cluster_attr_to_string((esp_zb_zcl_device_temp_config_attr_e)value);
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:                    return zb_constants_zcl_identify_cluster_attr_to_string((esp_zb_zcl_identify_attr_t)value);
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:                      
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:                      
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:                      
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG:        
    case ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:               
    case ESP_ZB_ZCL_CLUSTER_ID_ALARMS:                      
    case ESP_ZB_ZCL_CLUSTER_ID_TIME:                        
    case ESP_ZB_ZCL_CLUSTER_ID_RSSI_LOCATION:               
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT:                
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT:               
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE:                
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT:                
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_OUTPUT:               
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_VALUE:                
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_INPUT:                 
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_OUTPUT:                
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE:                 
    case ESP_ZB_ZCL_CLUSTER_ID_COMMISSIONING:               
    case ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE:                 
    case ESP_ZB_ZCL_CLUSTER_ID_POLL_CONTROL:                
    case ESP_ZB_ZCL_CLUSTER_ID_GREEN_POWER:                 
    case ESP_ZB_ZCL_CLUSTER_ID_KEEP_ALIVE:                  
    case ESP_ZB_ZCL_CLUSTER_ID_SHADE_CONFIG:                
    case ESP_ZB_ZCL_CLUSTER_ID_DOOR_LOCK:                   
    case ESP_ZB_ZCL_CLUSTER_ID_WINDOW_COVERING:             
    case ESP_ZB_ZCL_CLUSTER_ID_PUMP_CONFIG_CONTROL:         return UNKNOWN_SMART_CLUSTER_ATTR;
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT:                  return zb_constants_zcl_thermostat_cluster_attr_to_string((esp_zb_zcl_thermostat_attr_t)value);
    case ESP_ZB_ZCL_CLUSTER_ID_FAN_CONTROL:                 return UNKNOWN_SMART_CLUSTER_ATTR;
    case ESP_ZB_ZCL_CLUSTER_ID_DEHUMIDIFICATION_CONTROL /*ESP_ZB_ZCL_CLUSTER_ID_DEHUMID_CONTROL*/:             UNKNOWN_SMART_CLUSTER_ATTR;
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG:        return zb_constants_zcl_thermostat_ui_cluster_attr_to_string((esp_zb_zcl_thermostat_ui_config_attr_t)value);
    case ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL:               
    case ESP_ZB_ZCL_CLUSTER_ID_BALLAST_CONFIG:              
    case ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT:     
    case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:            
    case ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT:        
    case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:    
    case ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING:           
    case ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT:  
    case ESP_ZB_ZCL_CLUSTER_ID_PM2_5_MEASUREMENT:           
    case ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE:                    
    case ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT:      return UNKNOWN_SMART_CLUSTER_ATTR;
    case ESP_ZB_ZCL_CLUSTER_ID_METERING:                    return zb_constants_zcl_metering_cluster_attr_to_string((esp_zb_zcl_metering_attr_t)value);
    default:                                                return UNKNOWN_CLUSTER_ID;
  }
}

#ifdef __cplusplus
}
#endif
