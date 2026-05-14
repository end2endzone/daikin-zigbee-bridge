#pragma once

#include <stdint.h>
#include <stddef.h>
#include "DaikinEnums.h"

class DaikinSerialApi
{
public:
  // ---------------------------------------------------------------------------
  // ApiResult
  // Possible API Result code returned by all method calls.
  // ---------------------------------------------------------------------------
  typedef enum {
    API_RESULT_OK = 0,                  // Operation completed successfully.
    API_RESULT_UNKNOWN_ERROR,           // An unclassified error occurred on the listener side.
    API_RESULT_DAIKIN_INFO_PULL_FAIL,   // Listener could not retrieve data from the Daikin unit.
    API_RESULT_DAIKIN_INFO_PUSH_FAIL,   // Listener could not push settings to the Daikin unit.
    API_RESULT_TIMEOUT,                 // No response was received within the deadline (controller-side).
  } ApiResult;

  // ---------------------------------------------------------------------------
  // Shared constants
  // ---------------------------------------------------------------------------
  #define DAIKIN_IP_ADDRESS_SIZE            20
  #define DAIKIN_STATUS_MESSAGE_NAME_SIZE   32

  // Default synchronous call timeout (milliseconds).
  static constexpr unsigned long DEFAULT_TIMEOUT_MS = 5000UL;

  // ---------------------------------------------------------------------------
  // Daikin structures for the Serial Protocol API
  // ---------------------------------------------------------------------------
  
  typedef struct __attribute__((packed)) daikin_ip_address_s {
    char ip[DAIKIN_IP_ADDRESS_SIZE];  // Null-terminated IPv4 string, e.g. "192.168.1.42\0"
  } daikin_ip_address_t;

  // Daikin heatpump attributes: {
  //   Device name:  LivingRoom
  //   Power:        On
  //   Mode:         Heating
  //   Fan rate:     Level 5
  //   Fan dir:      Off
  //   Preset:       Error
  //   Target Temp:  1700
  //   Indoor Temp:  2250
  //   Outdoor Temp: 1800
  // };
  typedef struct __attribute__((packed)) daikin_status_info_s {
    char name[DAIKIN_STATUS_MESSAGE_NAME_SIZE];
    DaikinEnums::Power power;
    DaikinEnums::Mode mode;
    DaikinEnums::FanRate fan_rate;
    DaikinEnums::FanDir fan_dir;
    DaikinEnums::Preset preset;
    uint16_t target_temp;
    uint16_t indoor_temp;
    uint16_t outdoor_temp;
  } daikin_status_info_t;

  virtual ~DaikinSerialApi() {}

  // -------------------------------------------------------------------------
  // Public API
  // -------------------------------------------------------------------------

  // Request the IP address of the Daikin controller from the zigbee-bridge.
  // When the function is succesful, addr->ip is populated with a null-terminated IPv4 string.
  // Returns ApiResult::OK with a valid `addr` when succesful. Returns another result otherwise.
  virtual ApiResult getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) = 0;

  // Send a new target temperature to the Daikin controller
  // which forwards it to the Daikin unit.
  // Argument `temperature` is expressed in units of 0.01 °C (e.g. 2100 = 21.00 °C).
  virtual ApiResult setTargetTemperature(uint16_t temperature, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) = 0;

  // Request the full Daikin status of the Daikin controller from the zigbee-bridge.
  // Returns ApiResult::OK with a valid valid `status` when succesful. Returns another result otherwise.
  virtual ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) = 0;
};
