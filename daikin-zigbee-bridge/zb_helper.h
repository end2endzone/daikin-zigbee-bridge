#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "type_helper.h"
#include "logging.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "format_helper.h"
#include "zb_constants_helper.h"
#include "stelpro_constants.h"
#include "esp_mac.h"

typedef enum {
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_UNLOCK = 0x00,
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK1  = 0x01,
  ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_LOCK2  = 0x02,
} zb_zcl_thermostat_ui_config_keypad_lockout_t;

static const char* UNKNOWN_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_VALUE = "Unknown thermostat ui config keypad lockout value";
static const size_t ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN = (size_t)-1;
static const size_t ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE = (size_t)-2;

// Define the required buffer size to be able to convert any value to string.
// Format ESP_ZB_ZCL_ATTR_TYPE_64BITMAP is printed in binary and requires 'b' + 64 characters + '\0' = 66 bytes.
// Format ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY is 16 bytes printed as hexadecimal and requires '0x' + 32 digits + '\0' = 35 bytes
// Format ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING is 1 byte per character, 255 characters long string should be long enough for most use case 
static const size_t DATA_VALUE_STRING_BUFFER_SIZE = 256;

static const char* ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN = "Unknown Attribute Type";

typedef struct zb_attr_more_info_s {
  const char * unit;            // unit such as `°C`
  const char * scaled_unit;     // scaled unit such as `0.01°C`
  const char * min;             // minimum value of attribute
  const char * max;             // maximum value of attribute
  const char * notes;           // any textual notes
} zb_attr_more_info_t;

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
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:            return 8;
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
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:       return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:              return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:          return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:        return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:      return 2;
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:        return 4;
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:         return 8;
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:       return 16;
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:           return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
    default:                                     return ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN;
  }
}

static const char* zb_constants_zcl_attr_type_c_type_name(esp_zb_zcl_attr_type_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_TYPE_NULL:              return "void*";
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:              return "uint8_t";
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:             return "uint16_t";
    case ESP_ZB_ZCL_ATTR_TYPE_24BIT:             return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:             return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_40BIT:             return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_48BIT:             return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_56BIT:             return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_64BIT:             return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_BOOL:              return "bool";
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:           return "uint8_t";
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:          return "uint16_t";
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP:          return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:          return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP:          return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP:          return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP:          return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP:          return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U8:                return "uint8_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U16:               return "uint16_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U24:               return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U32:               return "uint32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U40:               return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U48:               return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U56:               return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_U64:               return "uint64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S8:                return "int8_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S16:               return "int16_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S24:               return "int32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S32:               return "int32_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S40:               return "int64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S48:               return "int64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S56:               return "int64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_S64:               return "int64_t";
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:         return "int8_t";
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:        return "int16_t";
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI:              return "float";
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE:            return "float";
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:            return "double";
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:      return "uint8_t*";
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:       return "char*";
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING: return "uint8_t*";
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:  return "char*";
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:             return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ARRAY:       return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT_ARRAY:       return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:         return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_SET:               return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:               return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:       return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:              return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:          return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:        return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:      return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:        return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:         return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:       return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:           return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
    default:                                     return ZB_ZCL_ATTR_TYPE_C_TYPE_UNKNOWN;
  }
}

static bool zb_zcl_attribute_data_pointer_to_string(char *buffer, size_t buffer_size, esp_zb_zcl_attr_type_t type_id, void *data_p) {
  static const char * DEFAULT_TO_STRING_VALUE = "???";

  if (!buffer || buffer_size == 0) {
    return false;
  }

  if (!data_p) {
    if (snprintf(buffer, buffer_size, "NULL") < 0)
      return false;
    return true;
  }

  switch (type_id) {
    case ESP_ZB_ZCL_ATTR_TYPE_NULL: {
      if (snprintf(buffer, buffer_size, "NULL") < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_U8:
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM: {
      uint8_t v = *(uint8_t *)data_p;
      if (snprintf(buffer, buffer_size, "%u", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_U16:
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM: {
      uint16_t v = *(uint16_t *)data_p;
      if (snprintf(buffer, buffer_size, "%u", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
    case ESP_ZB_ZCL_ATTR_TYPE_U32: {
      uint32_t v = *(uint32_t *)data_p;
      if (snprintf(buffer, buffer_size, "%u", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_S8: {
      int8_t v = *(int8_t *)data_p;
      if (snprintf(buffer, buffer_size, "%d", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_S16: {
      int16_t v = *(int16_t *)data_p;
      if (snprintf(buffer, buffer_size, "%d", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_S32: {
      int32_t v = *(int32_t *)data_p;
      if (snprintf(buffer, buffer_size, "%ld", v) < 0)
        return false;
      return true;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_BOOL: {
      bool v = *(bool *)data_p;
      const char * text = (v ? "true" : "false");
      if (snprintf(buffer, buffer_size, "%s", text) < 0)
        return false;
      return true;
    }
    break;

    // Bitmaps
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP     :
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP    :
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP    :    
    {
      // Get the size of the actual data
      size_t data_size = zb_constants_zcl_attr_type_size(type_id);
      if (data_size == ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE || data_size == ZB_ZCL_ATTR_TYPE_SIZE_UNKNOWN) {
        // Unable to get how long the data_p is.
        // Fallback with the unknwon value.
        snprintf(buffer, buffer_size, DEFAULT_TO_STRING_VALUE);
        return false;
      }

      // Output as hexedecimal
      bool success = toBin(data_p, data_size, buffer, buffer_size);
      return success;
    }
    break;


    // Print the following values as hexadecimal values
    case ESP_ZB_ZCL_ATTR_TYPE_24BIT       :
    case ESP_ZB_ZCL_ATTR_TYPE_40BIT       :
    case ESP_ZB_ZCL_ATTR_TYPE_48BIT       :
    case ESP_ZB_ZCL_ATTR_TYPE_56BIT       :
    case ESP_ZB_ZCL_ATTR_TYPE_64BIT       :
    case ESP_ZB_ZCL_ATTR_TYPE_U40         :
    case ESP_ZB_ZCL_ATTR_TYPE_U48         :
    case ESP_ZB_ZCL_ATTR_TYPE_U56         :
    case ESP_ZB_ZCL_ATTR_TYPE_U64         :
    case ESP_ZB_ZCL_ATTR_TYPE_S40         :
    case ESP_ZB_ZCL_ATTR_TYPE_S48         :
    case ESP_ZB_ZCL_ATTR_TYPE_S56         :
    case ESP_ZB_ZCL_ATTR_TYPE_S64         :
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI        :
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE      :
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE      :
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY :
    case ESP_ZB_ZCL_ATTR_TYPE_DATE        :
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME    :
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID  :
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID  :
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR   :
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY :
    {
      // Get the size of the actual data
      size_t data_size = zb_constants_zcl_attr_type_size(type_id);
      if (data_size == ZB_ZCL_ATTR_TYPE_SIZE_VARIABLE) {
        // Unable to get how long the data_p is.
        // Fail with the unknwon value.
        snprintf(buffer, buffer_size, DEFAULT_TO_STRING_VALUE);
        return false;
      }

      // Output as hexedecimal
      bool success = toHex(data_p, data_size, buffer, buffer_size);
      return success;
    }
    break;

    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:
      {
        uint8_t lenght = ((uint8_t *)data_p)[0]; // the first byte is always the length of the string
        const uint8_t *str = &((uint8_t *)data_p)[1]; // this is the actual string pointer (non-null terminated!)

        // Truncate the input string to match the size of the output buffer.
        if (lenght >= buffer_size)
          lenght = buffer_size - 1;

        // Copy the input string using memcpy because the string is non-null terminated.
        memcpy(buffer, str, lenght);
        buffer[lenght] = '\0'; // and the output string

        return true;
    }
    break;

    default:
      snprintf(buffer, buffer_size, DEFAULT_TO_STRING_VALUE);
      return false;
  }
}

static bool zb_zcl_attribute_to_string(char* buffer, size_t buffer_size, uint16_t cluster_id, esp_zb_zcl_attr_t * attr) {
  if (attr == nullptr)
    return false;

  const char * attr_name = zb_constants_smart_cluster_attr_to_string((esp_zb_zcl_cluster_id_t)cluster_id, attr->id);
  const char * attr_type_name = zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)attr->type);
  const char * attr_access_name = zb_constants_zcl_attr_access_to_string((esp_zb_zcl_attr_access_t)attr->access);

  // Compute the value of the data as a string
  char data_str[DATA_VALUE_STRING_BUFFER_SIZE];
  bool success = zb_zcl_attribute_data_pointer_to_string(data_str, DATA_VALUE_STRING_BUFFER_SIZE, (esp_zb_zcl_attr_type_t)attr->type, attr->data_p);

  int result = snprintf(buffer, buffer_size, "id=0x%04x (%s), type=0x%02x (%s), access=0x%02x (%s), manuf_code=0x%04x, data_p=0x%08x, data=%s",
      attr->id, attr_name, attr->type, attr_type_name, attr->access, attr_access_name, attr->manuf_code, attr->data_p, data_str);
  
  if (result < 0)
    return false;
  return true;
}

static TypeSign zb_constants_zcl_attr_type_signed(esp_zb_zcl_attr_type_t value) {
  switch (value) {
    case ESP_ZB_ZCL_ATTR_TYPE_NULL:              return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT:              return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_24BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_40BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_48BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_56BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_64BIT:             return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_BOOL:              return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:           return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_24BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_40BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_48BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_56BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_64BITMAP:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U8:                return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U16:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U24:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U32:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U40:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U48:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U56:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_U64:               return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S8:                return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S16:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S24:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S32:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S40:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S48:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S56:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_S64:               return TYPE_SIGN_SIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:         return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:        return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_SEMI:              return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_SINGLE:            return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_DOUBLE:            return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING:      return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING:       return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING: return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING:  return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_ARRAY:             return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ARRAY:       return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_32BIT_ARRAY:       return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE:         return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_SET:               return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_BAG:               return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY:       return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_DATE:              return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME:          return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID:        return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID:      return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID:        return TYPE_SIGN_UNKNOWN;
    case ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR:         return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY:       return TYPE_SIGN_UNSIGNED;
    case ESP_ZB_ZCL_ATTR_TYPE_INVALID:           return TYPE_SIGN_UNKNOWN;
    default:                                     return TYPE_SIGN_UNKNOWN;
  }
}

template<typename T>
static bool zb_assert_attribute_size(const char* function, esp_zb_zcl_attr_type_t type_id) {
  size_t attr_size = zb_constants_zcl_attr_type_size(type_id);
  size_t value_size = sizeof(T);
  if (value_size != attr_size) {
    const char * attr_type_name = zb_constants_zcl_attr_type_to_string(type_id);
    const char * value_type_name = typeString<T>();
    log_e("*** Size assertion failed in %s() ! Attribute type '%s' is %d bit does not match value type '%s' which is %d bit.",
      function,
      attr_type_name,
      attr_size*8,
      value_type_name,
      value_size*8
      );
    return false;
  }
  return true;
}

template<typename T>
static bool zb_assert_attribute_sign(const char* function, esp_zb_zcl_attr_type_t type_id) {
  TypeSign attr_sign = zb_constants_zcl_attr_type_signed(type_id);
  TypeSign value_sign = typeSign<T>();
  if (value_sign != attr_sign) {
    const char * attr_type_name = zb_constants_zcl_attr_type_to_string(type_id);
    const char * value_type_name = typeString<T>();
    log_e("*** Sign assertion failed in %s() ! Attribute type '%s' is '%s' does not match value type '%s' which is '%s'.",
      function,
      attr_type_name,
      ::toString(attr_sign),
      value_type_name,
      ::toString(value_sign)
      );
    return false;
  }
  return true;
}

static bool zb_zcl_attribute_is_sentinel(esp_zb_zcl_attr_t* attr) {
  // The ESP Zigbee SDK uses a sentinel node (dummy node) at the beginning of each attribute list to simplify list operations.
  // This dummy node:
  // * Has all fields zeroed (id=0x0000, type=0x00, access=0x00, data_p=NULL)
  // * Is not an actual attribute
  // * Simplifies insert/delete operations (no special case for empty lists)
  // * The next pointer points to the first real attribute

  if (attr == nullptr)
    return false;

  static const esp_zb_zcl_attr_t ZB_ZCL_SENTINEL_ATTRIBUTE = {0};
  bool is_sentinel = (memcmp(attr, &ZB_ZCL_SENTINEL_ATTRIBUTE, sizeof(ZB_ZCL_SENTINEL_ATTRIBUTE)) == 0);
  return is_sentinel;
}

static bool zb_zcl_cluster_is_sentinel(esp_zb_zcl_cluster_t* cluster) {
  // The ESP Zigbee SDK uses a sentinel node (dummy node) at the beginning of each attribute list to simplify list operations.
  // This dummy node:
  // * Has all fields zeroed (id=0x0000, type=0x00, access=0x00, data_p=NULL)
  // * Is not an actual attribute
  // * Simplifies insert/delete operations (no special case for empty lists)
  // * The next pointer points to the first real attribute

  if (cluster == nullptr)
    return false;

  static const esp_zb_zcl_cluster_t ZB_ZCL_SENTINEL_CLUSTER = {0};
  bool is_sentinel = (memcmp(cluster, &ZB_ZCL_SENTINEL_CLUSTER, sizeof(ZB_ZCL_SENTINEL_CLUSTER)) == 0);
  return is_sentinel;
}

static esp_zb_zcl_attr_t* zb_find_attribute_in_attribute_list(esp_zb_attribute_list_t* list, uint16_t attribute_id)
{
  esp_zb_attribute_list_t *element = list;

  // Skip the sentinel/dummy head node, if present
  if (element != nullptr && zb_zcl_attribute_is_sentinel(&element->attribute)) {
    element = element->next;  // Skip node
  }

  while (element != nullptr) {
    if (element->attribute.id == attribute_id) {
      return &element->attribute;
    }

    // Next attribute element in attribute list
    element = element->next;
  }
  return nullptr; // Not found
}

static esp_zb_zcl_attr_t* zb_find_attribute_in_cluster_list(esp_zb_cluster_list_t* list, uint16_t cluster_id, uint16_t attribute_id)
{
  esp_zb_cluster_list_t *element = list;

  // Skip sentinel/dummy head node, if present
  if (element != nullptr && zb_zcl_cluster_is_sentinel(&element->cluster)) {
    element = element->next;  // Skip node
  }

  while (element != nullptr) {
    esp_zb_zcl_cluster_t *cluster = &element->cluster;

    // Is that the cluster we are looking for ?
    if (cluster != nullptr && cluster->cluster_id == cluster_id) {

      // Only handle clusters that use the linked-list attribute model
      esp_zb_attribute_list_t *attr_list = cluster->attr_list;
      if (cluster->attr_count == 0 && attr_list != nullptr) {
        esp_zb_zcl_attr_t *attr = zb_find_attribute_in_attribute_list(attr_list, attribute_id);
        if (attr != nullptr) {
          return attr;  // Found
        }
      }
    }

    // Next cluster element in cluster list
    element = element->next;
  }

  return nullptr; // Not found in any cluster
}

template<typename T>
static bool zb_set_attribute_value_in_attr_list(esp_zb_attribute_list_t *list, uint16_t attribute_id, const T &new_value)
{
  esp_zb_zcl_attr_t *attr = zb_find_attribute_in_attribute_list(list, attribute_id);
  if (attr == nullptr || attr->data_p == nullptr) {
    return false;
  }

  // Assert correct size
  if (!zb_assert_attribute_size<T>(__FUNCTION__, (esp_zb_zcl_attr_type_t)attr->type))
    return false;

  // Assert correct sign
  if (!zb_assert_attribute_sign<T>(__FUNCTION__, (esp_zb_zcl_attr_type_t)attr->type))
    return false;

  memcpy(attr->data_p, &new_value, sizeof(T));
  return true;
}

template<typename T>
static bool zb_set_attribute_value_in_cluster_list(esp_zb_cluster_list_t *list, uint16_t cluster_id, uint16_t attribute_id, const T &new_value)
{
  esp_zb_zcl_attr_t *attr = zb_find_attribute_in_cluster_list(list, cluster_id, attribute_id);
  if (attr == nullptr || attr->data_p == nullptr) {
    return false;
  }

  // Assert correct size
  if (!zb_assert_attribute_size<T>(__FUNCTION__, (esp_zb_zcl_attr_type_t)attr->type))
    return false;

  // Assert correct sign
  if (!zb_assert_attribute_sign<T>(__FUNCTION__, (esp_zb_zcl_attr_type_t)attr->type))
    return false;

  memcpy(attr->data_p, &new_value, sizeof(T));
  return true;
}

static const zb_attr_more_info_t * zb_get_attribute_more_info(uint16_t cluster_id, uint16_t attribute_id) {
  if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY) {
    if (attribute_id == ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID) {
      static constexpr zb_attr_more_info_t more = {
        .notes = "Duration in seconds the device stays in identify mode.",
      };
      return &more;
    }
  } else if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT) {
    switch (attribute_id) {
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID: {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
          .notes = "Value `0x8000` means _Not Available_.",
        };
        return &more;
      }
      break;
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID: {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
          .min = "500",
          .max = "3500",
          .notes = "Set artificially high so it never constrains the heating setpoint.",
        };
        return &more;
      }
      break;
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID: {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
          .min = "500",
          .max = "3000",
          .notes = "Writing a value outside the valid range will result in a INVALID_VALUE ZCL status. Must remain below Occupied Cooling Setpoint to avoid ZCL enforcement errors.",
        };
        return &more;
      }
      break;
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID: {
        static constexpr zb_attr_more_info_t more = {
          .notes = "Changing this attribute also synchronises StelproSystemMode (0x401C). Both carry identical semantics and are always kept in sync: writing either one causes the other to be updated immediately.",
        };
        return &more;
      }
      break;
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID: {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
          .notes = "ZCL standard outdoor temperature, intended for a physical sensor on the device.",
        };
        return &more;
      }
      break;
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID: {
        static constexpr zb_attr_more_info_t more = {
          .unit = "%",
          .min = "0",
          .max = "100",
          .notes = "Percentage of heating demand. Must not be set to a non-zero value unless `running_state` has the `HEAT` bit set. Must be reset to `0` before clearing the `HEAT` bit. Zigbee2MQTT assumes range `[0, 255]` but this implementation uses `[0, 100]`.",
        };
        return &more;
      }
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID:
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
        };
        return &more;
      }
      break;
      case ZB_STELPRO_ATTR_OUTDOOR_TEMP_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .unit = "°C",
          .scaled_unit = "0.01°C",
          .min = "-3200",
          .max = "19900",
          .notes = "Outdoor temperature displayed on the thermostat face.",
        };
        return &more;
      }
      break;
      case ZB_STELPRO_ATTR_SYSTEM_MODE_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .notes = "Mirror of the standard `SystemMode` attribute (`0x001C`). Both carry identical semantics and are always kept in sync: writing either one causes the other to be updated immediately.",
        };
        return &more;
      }
      break;
      case ZB_STELPRO_ATTR_POWER_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .unit = "W",
          .min = "0",
          .max = "4000",
          .notes = "Instantaneous electrical power draw of the baseboard heater. Updated at runtime from the heating demand calculation. Zigbee2MQTT exposes this value directly in Watts.",
        };
        return &more;
      }
      break;
      case ZB_STELPRO_ATTR_ENERGY_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .unit = "Wh",
          .notes = "Cumulative energy consumption since last reset. The on-wire value is in **Watt-hours (Wh)**. Zigbee2MQTT divides by `1000` before publishing, so Home Assistant receives the value in **kWh**.",
        };
        return &more;
      }
      break;
      case ZB_STELPRO_ATTR_PEAK_DEMAND_ICON_ID:
      {
        static constexpr zb_attr_more_info_t more = {
          .unit = "seconds",
          .scaled_unit = "",
          .min = "0",
          .max = "64800",
          .notes = "Set peak demand event icon for the specified number of seconds.",
        };
        return &more;
      }
    }
  }
  
  // Nothing special for this attribute
  return nullptr;
}

typedef uint8_t esp_48bit_addr_t[6];
typedef uint8_t esp_64bit_addr_t[8];
typedef uint8_t esp_eui64_addr_t[8];

/**
 * @brief Convert a 64bit esp_zb_ieee_addr_t address to its hex representation in a String.
 * For example: 0x0123456789abcdef.
 * ZBOSS stores 64-bit of IEEE long address in little-endian order internally, so byte[0] is the LSB.
 * See functions `esp_zb_set_long_address()` and `esp_zb_get_long_address()` from `esp-zigbee-lib\include\nwk\esp_zigbee_nwk.h`.
 * @param addr  Pointer to bytes containing the address.
 */
static String zb_ieee_long_addr_to_string(esp_zb_ieee_addr_t addr) {
  char buffer[24] = {0};

  // Print characters in reverse order.
  snprintf(buffer, sizeof(buffer), "0x%02x%02x%02x%02x%02x%02x%02x%02x", 
    addr[7],
    addr[6],
    addr[5],
    addr[4],
    addr[3],
    addr[2],
    addr[1],
    addr[0]);

  return String(buffer);    
}

/**
 * @brief Convert a 48bit base MAC address to its hex representation in a String.
 * For example: 0x0123456789ab.
 * Value is in big-endian order internally, so byte[0] is the MSB.
 * See functions `esp_read_mac()`, `esp_base_mac_addr_set()` (deprecated) and `esp_iface_mac_addr_set()` from ``.
 * @param addr  Pointer to bytes containing the address.
 */
static String esp_base_ieee_addr_to_string(esp_48bit_addr_t addr) {
  char buffer[24] = {0};

  // Print characters in natural order.
  snprintf(buffer, sizeof(buffer), "0x%02x%02x%02x%02x%02x%02x", 
    addr[0],
    addr[1],
    addr[2],
    addr[3],
    addr[4],
    addr[5]);

  return String(buffer);    
}

/**
 * @brief Convert a 64bit base MAC address to its hex representation in a String.
 * For example: 0x0123456789abcdef.
 * Value is in big-endian order internally, so byte[0] is the MSB.
 * See functions `esp_read_mac()`, `esp_base_mac_addr_set()` (deprecated) and `esp_iface_mac_addr_set()` from ``.
 * @param addr  Pointer to bytes containing the address.
 */
static String esp_eui64_addr_to_string(esp_eui64_addr_t addr) {
  char buffer[24] = {0};

  // Print characters in natural order.
  snprintf(buffer, sizeof(buffer), "0x%02x%02x%02x%02x%02x%02x%02x%02x", 
    addr[0],
    addr[1],
    addr[2],
    addr[3],
    addr[4],
    addr[5],
    addr[6],
    addr[7]);

  return String(buffer);    
}

static void zb_ieee_addr_set_oui(const uint8_t target_oui[3]) {
  // From https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/misc_system_api.html#mac-address
  // In ESP-IDF, the MAC addresses for the various network interfaces are calculated from a single base MAC address.
  // By default, the Espressif base MAC address is used. This base MAC address is pre-programmed into the 
  // ESP32 eFuse in the factory during production.
  //
  //   | Interface     | MAC Address (4 universally administered, default) | MAC Address (2 universally administered)   |
  //   |---------------|---------------------------------------------------|--------------------------------------------|
  //   | Wi-Fi Station | base_mac                                          | base_mac                                   |
  //   | Wi-Fi SoftAP  | base_mac, +1 to the last octet                    | Local MAC (derived from Wi-Fi Station MAC) |
  //   | Bluetooth     | base_mac, +2 to the last octet                    | base_mac, +1 to the last octet             |
  //   | Ethernet      | base_mac, +3 to the last octet                    | Local MAC (derived from Bluetooth MAC)     |
  //

  // From esp32\3.3.7\libraries\ESP32\examples\MacAddress\GetMacAddress\GetMacAddress.ino :
  // esp_mac_type_t values:
  //   ESP_MAC_WIFI_STA - MAC for WiFi Station (6 bytes)
  //   ESP_MAC_WIFI_SOFTAP - MAC for WiFi Soft-AP (6 bytes)
  //   ESP_MAC_BT - MAC for Bluetooth (6 bytes)
  //   ESP_MAC_ETH - MAC for Ethernet (6 bytes)
  //   ESP_MAC_IEEE802154 - if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC for IEEE802154 (8 bytes)
  //   ESP_MAC_BASE - Base MAC for that used for other MAC types (6 bytes)
  //   ESP_MAC_EFUSE_FACTORY - MAC_FACTORY eFuse which was burned by Espressif in production (6 bytes)
  //   ESP_MAC_EFUSE_CUSTOM - MAC_CUSTOM eFuse which was can be burned by customer (6 bytes)
  //   ESP_MAC_EFUSE_EXT - if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC_EXT eFuse which is used as an extender for IEEE802154 MAC (2 bytes)

  // From https://github.com/espressif/esp-idf/blob/master/examples/system/base_mac_address/main/base_mac_address_example_main.c#L67
  // From https://github.com/espressif/esp-idf/tree/v5.5.3/examples/system/base_mac_address

  esp_err_t ret = ESP_OK;

  // Get base MAC address from eFuse BLK0 (default option, burned by Espressif in production).
  // Value is 6 bytes long, in big-endian order internally, so byte[0] is the MSB.
  uint8_t base_mac_addr[6] = {0};
  ret = esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_FACTORY);
  if (ret != ESP_OK) {
    log_e("Failed to get base MAC address from eFuse BLK0!");
    logError(ret);
    log_e("Aborting");
    abort();
  }
  log_i("Base MAC Address (EFUSE BLK0): %s", esp_base_ieee_addr_to_string(base_mac_addr).c_str());

  // Get EUI-64 zigbee address.
  // Value is 8 bytes long, in big-endian order internally, so byte[0] is the MSB.
  //
  // Address layout (big-endian) in relation to base mac address:
  //   Bytes [0..2] = OUI  (base_mac[0] XOR 0x02, base_mac[1], base_mac[2])
  //   Bytes [3..4] = 0xFF, 0xFE  (FF:FE insertion)
  //   Bytes [5..6] = base_mac[3], base_mac[4]
  //   Bytes [7]    = base_mac[5] + 3  (ESP_MAC_IEEE802154 offset)
  esp_eui64_addr_t zigbee_addr = {0};
  ret = esp_read_mac(zigbee_addr, ESP_MAC_IEEE802154);
  if (ret != ESP_OK) {
    log_e("Failed to get zigbee MAC address!");
    logError(ret);
    log_e("Aborting");
    abort();
  }
  log_i("Default Zigbee address: %s", esp_eui64_addr_to_string(zigbee_addr).c_str());

  // Apply the OUI (create a derived EUI-64 zigbee address)
  zigbee_addr[0] = 0xBC;
  zigbee_addr[1] = 0x33;
  zigbee_addr[2] = 0xAC;

  // Set a new (derived?) EUI-64 zigbee address.
  log_i("Changing Zigbee address to %s", esp_eui64_addr_to_string(zigbee_addr).c_str());
  ret = esp_iface_mac_addr_set(zigbee_addr, ESP_MAC_IEEE802154);
  if (ret != ESP_OK) {
    log_e("Failed to set zigbee MAC address!");
    logError(ret);
    log_e("Aborting");
    abort();
  }

  // Confirm the change
  esp_eui64_addr_t derived_zigbee_addr = {0};
  ret = esp_read_mac(derived_zigbee_addr, ESP_MAC_IEEE802154);
  if (ret != ESP_OK) {
    log_e("Failed to get derived zigbee MAC address!");
    logError(ret);
    log_e("Aborting");
    abort();
  }
  log_i("Derived Zigbee address: %s", esp_eui64_addr_to_string(derived_zigbee_addr).c_str());
}
