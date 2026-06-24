#pragma once

#include "Zigbee.h"
#include "esp_zigbee_type.h"
#include <stdint.h>

// File `esp_zigbee_type.h` of library `esp-zigbee-sdk` does not define custom type `zb_uint8_t`.
// However, files such as `zcl\esp_zigbee_zcl_thermostat_ui_config.h` refers to type `zb_uint8_t` in definition of `ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_SCHEDULE_PROGRAMMING_VISIBILITY_DEFAULT_VALUE` :
// `#define ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_SCHEDULE_PROGRAMMING_VISIBILITY_DEFAULT_VALUE ((zb_uint8_t)0x00)`.
//
// This file defines a custom type named `zb_uint8_t` to facilitate compilation and use of esp-zigbee-sdk include files.

/*
typedef struct esp_zb_uint8_s {
    uint8_t low;
} ESP_ZB_PACKED_STRUCT
zb_uint8_t;
*/

typedef uint8_t zb_uint8_t;
