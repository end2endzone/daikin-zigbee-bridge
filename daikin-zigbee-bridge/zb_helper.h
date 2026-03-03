#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "type_helper.h"
#include "logging.h"
#include "zb_constants_helper.h"

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
    logEntry("ERROR: *** Size assertion failed in %s() ! Attribute type '%s' is %d bit does not match value type '%s' which is %d bit.",
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
    logEntry("ERROR: *** Sign assertion failed in %s() ! Attribute type '%s' is '%s' does not match value type '%s' which is '%s'.",
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
