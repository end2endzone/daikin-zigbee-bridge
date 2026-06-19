#pragma once

#include <stdint.h>
#include <stddef.h>
#include "DaikinEnums.h"
#include "format_helper.h"
#include "logging.h"

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

  // Converts a ApiResult enum to a String
  static String toString(ApiResult value) {
    switch (value) {
      case API_RESULT_OK                   : return "OK"                   ;
      case API_RESULT_UNKNOWN_ERROR        : return "UNKNOWN_ERROR"        ;
      case API_RESULT_DAIKIN_INFO_PULL_FAIL: return "DAIKIN_INFO_PULL_FAIL";
      case API_RESULT_DAIKIN_INFO_PUSH_FAIL: return "DAIKIN_INFO_PUSH_FAIL";
      case API_RESULT_TIMEOUT              : return "TIMEOUT"              ;
      default:            return "Error";
    }
  }

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
  //   Comp Freq:    0
  // };
  typedef struct __attribute__((packed)) daikin_status_info_s {
    char name[DAIKIN_STATUS_MESSAGE_NAME_SIZE];
    DaikinEnums::Power power;
    DaikinEnums::Mode mode;
    DaikinEnums::FanRate fan_rate;
    DaikinEnums::FanDir fan_dir;
    DaikinEnums::Preset preset;
    int16_t target_temp;
    int16_t indoor_temp;
    int16_t outdoor_temp;
    uint8_t compressor_freq;
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
  virtual ApiResult setTargetTemperature(int16_t temperature, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) = 0;

  // Request the full Daikin status of the Daikin controller from the zigbee-bridge.
  // Returns ApiResult::OK with a valid valid `status` when succesful. Returns another result otherwise.
  virtual ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) = 0;

  // -------------------------------------------------------------------------
  // Public helper functions
  // -------------------------------------------------------------------------

  /**
   * @brief Check if an indoor unit is currently heating based on the given Daikin status info structure.
   */
  static bool isHeating(const daikin_status_info_t *status) {
    if (status == nullptr) return false;
    if (status->power != DaikinEnums::Power::POWER_ON)
      return false;
    if (status->compressor_freq == 0)
      return false;
    switch(status->mode) {
      case DaikinEnums::Mode::MODE_AUTO:
        return (status->target_temp > status->indoor_temp); // depends on setpoint vs indoor temperature
        break;
      case DaikinEnums::Mode::MODE_HEATING:
        return true;
        break;
      default:
        return false;
    }
    return false;
  }

  /**
   * @brief Check if an indoor unit is currently cooling based on the given Daikin status info structure.
   */
  static bool isCooling(const daikin_status_info_t *status) {
    if (status == nullptr) return false;
    if (status->power != DaikinEnums::Power::POWER_ON)
      return false;
    if (status->compressor_freq == 0)
      return false;
    switch(status->mode) {
      case DaikinEnums::Mode::MODE_AUTO:
        return (status->target_temp < status->indoor_temp); // depends on setpoint vs indoor temperature
        break;
      case DaikinEnums::Mode::MODE_COOLING:
        return true;
        break;
      default:
        return false;
    }
    return false;
  }

  /**
   * @brief Get an estimated instantaneous electrical power consumption in Watts from the given Daikin status info structure.
   * The returned value combines the power consumption of both of indoor and output units.
   * Returns a value is in Watts.
   * The function assume a single indoor units connected to the outdoor unit.
   * Change num_indoor_unit accordingly if the outdoor unit is connected to multiple indoor units.
   * For example, for a heat pump system with 3 indoor units, the total power consumption for a single indoor unit is calculated from:
   *   1. The  indoor unit power consumption based on its fan speed setting
   *   2. The outdoor unit power consumption based on the compressor frequency, multipled by 1/3.
   * For  indoor unit, the estimation is based on the fan speed setting.  
   * For outdoor unit, the estimation is based on the compressor frequency.
   */
  static uint16_t getEstimatedInstantaneousPower(const daikin_status_info_t *status, int num_indoor_unit = 1) {
    if (status == nullptr) return 0;

    #define INDOOR_FAN_TOTAL_W 48.0 // based on the specification label sticker
    #define OUTDOOR_COMPRESSOR_TOTAL_W 1500.0 // based on the specification label sticker, 18000 BTU unit
    #define MIN_COMPRESSOR_FREQ_HZ  0.0 // based on observations
    #define MAX_COMPRESSOR_FREQ_HZ 71.0 // based on observations

    // Compute indoor unit power based on fan speed
    float indoor_fan_ratio = 0.0;
    switch (status->fan_rate) {
      case DaikinEnums::FanRate::FAN_QUIET:   indoor_fan_ratio = 0.10; break; //  10%
      case DaikinEnums::FanRate::FAN_LEVEL1:  indoor_fan_ratio = 0.25; break; //  25%
      case DaikinEnums::FanRate::FAN_LEVEL2:  indoor_fan_ratio = 0.40; break; //  40%
      case DaikinEnums::FanRate::FAN_LEVEL3:  indoor_fan_ratio = 0.60; break; //  60%
      case DaikinEnums::FanRate::FAN_LEVEL4:  indoor_fan_ratio = 0.80; break; //  80%
      case DaikinEnums::FanRate::FAN_LEVEL5:  indoor_fan_ratio = 1.00; break; // 100%
      default:                                indoor_fan_ratio = 0.00; break; //   0%
    }
    uint16_t indoor_unit_power = (uint16_t)(indoor_fan_ratio * INDOOR_FAN_TOTAL_W);
    
    // Compute outdoor unit power based on compressor frequency
    float outdoor_compressor_ratio = (float)map(
        (float)status->compressor_freq,
        MIN_COMPRESSOR_FREQ_HZ,
        MAX_COMPRESSOR_FREQ_HZ,
        0.0,
        1.0);
    float outoor_unit_share_factor = 1.0/(float)num_indoor_unit;
    uint16_t outoor_unit_power = (uint16_t)(outdoor_compressor_ratio * OUTDOOR_COMPRESSOR_TOTAL_W * outoor_unit_share_factor);

    uint16_t total_power = indoor_unit_power + outoor_unit_power;
    return total_power;
  }

  static String toString(const daikin_status_info_t *status, int num_indoor_unit = 1) {
    String output;
    if (status == nullptr) return output;

    output.reserve(320); // the default sample output is 304 characters long

    output += "{\n";

    // Print basic info
    {
      output += strformat("  Device name:  %s\n", status->name);
    }
    
    // Print control info
    {
      output += strformat("  Power:        %s\n", DaikinEnums::toString(status->power).c_str());
      output += strformat("  Mode:         %s\n", DaikinEnums::toString(status->mode).c_str());
      output += strformat("  Fan rate:     %s\n", DaikinEnums::toString(status->fan_rate).c_str());
      output += strformat("  Fan dir:      %s\n", DaikinEnums::toString(status->fan_dir).c_str());
      output += strformat("  Preset:       %s\n", DaikinEnums::toString(status->preset).c_str());
      output += strformat("  Target Temp:  %.1f°C\n", status->target_temp / 100.0);
    }

    // Print sensor info
    {
      output += strformat("  Indoor Temp:  %.1f°C\n", status->indoor_temp / 100.0);
      output += strformat("  Outdoor Temp: %.1f°C\n", status->outdoor_temp / 100.0);
      output += strformat("  Comp. Freq:   %u Hz\n", status->compressor_freq);
    }

    // Print derived information
    {
      bool heating = isHeating(status);
      bool cooling = isCooling(status);
      uint16_t instantaneous_power = getEstimatedInstantaneousPower(status, num_indoor_unit);
      output += strformat("  Heating:      %s\n", bool2str(heating));
      output += strformat("  Cooling:      %s\n", bool2str(cooling));
      output += strformat("  Consumption:  %u W\n", instantaneous_power);
    }

    // Do not add a newline character at the end of the output string.
    // Implement the same behavior as any other toString() function which output a single line string
    // (it does not add newline characters at all).
    output += "}";

    return output;
  }
};
