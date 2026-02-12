#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"

typedef enum {
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_UNLOCK = 0x00,
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK1  = 0x01,
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK2  = 0x02,
} zb_zcl_thermostat_ui_config_keypad_lockout_t;

static const char* UNKNOWN_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_VALUE = "Unknown thermostat ui config keypad lockout value";
static const size_t ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN = (size_t)-1;
static const size_t ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE = (size_t)-2;

static const char* zb_zcl_thermostat_ui_config_keypad_lockout_to_string(zb_zcl_thermostat_ui_config_keypad_lockout_t value) {
  switch (value) {
    case ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_UNLOCK:    return "Unlock";
    case ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK1:     return "Lock-1";
    case ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK2:     return "Lock-2";
    default:                                                        return UNKNOWN_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_VALUE;
  }
}

static size_t zb_constants_zcl_attr_type_size(esp_zb_zcl_attr_type_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_TYPE_NULL:              return 0;
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:              return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:             return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_24BIT:             return 3;
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:             return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_40BIT:             return 5;
    case ESP_ZB_ZCL_ATTR_TYPE_48BIT:             return 6;
    case ESP_ZB_ZCL_ATTR_TYPE_56BIT:             return 7;
    case ESP_ZB_ZCL_ATTR_TYPE_64BIT:             return 8;
    case ESP_ZB_ZCL_ATTR_TYPE_BOOL:              return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:           return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:          return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP:          return 3;
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:          return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP:          return 5;
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP:          return 6;
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP:          return 7;
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP:          return 8;
    case ESP_ZB_ZCL_ATTR_TYPE_U8:                return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_U16:               return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_U24:               return 3;
    case ESP_ZB_ZCL_ATTR_TYPE_U32:               return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_U40:               return 5;
    case ESP_ZB_ZCL_ATTR_TYPE_U48:               return 6;
    case ESP_ZB_ZCL_ATTR_TYPE_U56:               return 7;
    case ESP_ZB_ZCL_ATTR_TYPE_U64:               return 8;
    case ESP_ZB_ZCL_ATTR_TYPE_S8:                return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_S16:               return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_S24:               return 3;
    case ESP_ZB_ZCL_ATTR_TYPE_S32:               return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_S40:               return 5;
    case ESP_ZB_ZCL_ATTR_TYPE_S48:               return 6;
    case ESP_ZB_ZCL_ATTR_TYPE_S56:               return 7;
    case ESP_ZB_ZCL_ATTR_TYPE_S64:               return 8;
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:         return 1;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:        return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI:              return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE:            return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:            return 5;
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:      return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:       return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING: return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:  return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:             return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ARRAY:       return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT_ARRAY:       return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:         return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_SET:               return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:               return ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE;
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:       return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:              return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:          return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:        return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:      return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:        return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:         return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:       return 16;
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:           return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    default:                                     return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
  }
}
