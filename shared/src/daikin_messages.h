#pragma once

#include <stdint.h>
#include <stddef.h>

enum MessageIds {
  DAIKIN_MESSAGE_UNKNOWN_ID = 0,
  DAIKIN_MESSAGE_GET_IP_ADDRESS_REQUEST_ID,
  DAIKIN_MESSAGE_GET_IP_ADDRESS_RESPONSE_ID,
  DAIKIN_MESSAGE_SET_TARGET_TEMPERATURE_REQUEST_ID,
  DAIKIN_MESSAGE_SET_TARGET_TEMPERATURE_RESPONSE_ID,
  DAIKIN_MESSAGE_GET_STATUS_REQUEST_ID,
  DAIKIN_MESSAGE_GET_STATUS_RESPONSE_ID,
};

#define DAIKIN_STATUS_MESSAGE_NAME_SIZE 30
#define DAIKIN_IP_ADDRESS_MESSAGE_IP_ADDRESS_SIZE 20


// DAIKIN_MESSAGE_GET_IP_ADDRESS_REQUEST_ID
typedef struct daikin_get_ip_address_request_s {
} daikin_get_ip_address_request_t;

// DAIKIN_MESSAGE_GET_IP_ADDRESS_RESPONSE_ID
typedef struct daikin_get_ip_address_response_s {
  char ip[DAIKIN_IP_ADDRESS_MESSAGE_IP_ADDRESS_SIZE];
} daikin_get_ip_address_response_t;



// DAIKIN_MESSAGE_SET_TARGET_TEMPERATURE_REQUEST_ID
typedef struct daikin_set_target_temperature_request_s {
  uint16_t temp;
} daikin_set_target_temperature_request_t;

// DAIKIN_MESSAGE_SET_TARGET_TEMPERATURE_RESPONSE_ID
typedef struct daikin_set_target_temperature_response_s {
  uint16_t temp;
} daikin_set_target_temperature_response_t;



// DAIKIN_MESSAGE_GET_STATUS_REQUEST_ID
typedef struct daikin_get_status_request_s {
} daikin_get_status_request_t;

// DAIKIN_MESSAGE_GET_STATUS_RESPONSE_ID
//
// Daikin heatpump attributes: {
//   Device name:  LivingRoom
//   Power:        On
//   Mode:         Heating
//   Fan rate:     Level 5
//   Fan dir:      Off
//   Preset:       Error
//   Target Temp:  17.00
//   Indoor Temp:  22.50
//   Outdoor Temp: 18.00
// };
typedef struct daikin_get_status_response_s {
  char name[DAIKIN_STATUS_MESSAGE_NAME_SIZE];
  int power;
  int mode;
  int fan_rate;
  int fan_dir;
  int preset;
  uint16_t target_temp;
  uint16_t indoor_temp;
  uint16_t outdoor_temp;
} daikin_get_status_response_t;
