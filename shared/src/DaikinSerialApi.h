#pragma once

#include <stdint.h>
#include <stddef.h>

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
  //   Target Temp:  17.00
  //   Indoor Temp:  22.50
  //   Outdoor Temp: 18.00
  // };
  typedef struct __attribute__((packed)) daikin_status_info_s {
    char name[DAIKIN_STATUS_MESSAGE_NAME_SIZE];
    int power;
    int mode;
    int fan_rate;
    int fan_dir;
    int preset;
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
  ApiResult getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms);

  // Send a new target temperature to the Daikin controller
  // which forwards it to the Daikin unit.
  // Argument `temperature` is expressed in units of 0.01 °C (e.g. 2100 = 21.00 °C).
  ApiResult setTargetTemperature(uint16_t temperature, unsigned long timeout_ms);

  // Request the full Daikin status of the Daikin controller from the zigbee-bridge.
  // Returns ApiResult::OK with a valid valid `status` when succesful. Returns another result otherwise.
  ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms);
};
