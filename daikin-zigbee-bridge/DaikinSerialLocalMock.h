#pragma once

#include <Arduino.h>
#include "DaikinSerialApi.h"
#include <SoftTimers.h>

// Temperature simulation timing
#define SIMULATION_UPDATE_INTERVAL            5000  //  5.0 seconds
#define SIMULATION_DEFAULT_ROOM_TEMPERATURE   2000  // 20.0°C
#define SIMULATION_DEFAULT_HEATING_SETPOINT   2400  // 24.0°C
#define SIMULATION_TEMPERATURE_DIFF_HIGH       500  //  5.0°C
#define SIMULATION_TEMPERATURE_STEP_LOW       (1 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_STEP_LOW       (1 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_STEP_HIGH      (5 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)

#define SIMULATION_COMPRESSOR_FREQ_IDLE          0
#define SIMULATION_COMPRESSOR_FREQ_LOW          25
#define SIMULATION_COMPRESSOR_FREQ_HIGH         71

class DaikinSerialLocalMock : public DaikinSerialApi
{
private:
  DaikinSerialApi::daikin_ip_address_t _ipaddr;
  DaikinSerialApi::daikin_status_info_t _status;
  SoftTimer tempSimulationUpdateTimer;

public:
  DaikinSerialLocalMock() {
    snprintf(_ipaddr.ip, sizeof(DaikinSerialApi::daikin_ip_address_t::ip), "%s", "101.102.103.104");

    //   Device name:  LivingRoom
    //   Power:        On
    //   Mode:         Heating
    //   Fan rate:     Level 5
    //   Fan dir:      Off
    //   Preset:       Error
    //   Target Temp:  1700
    //   Indoor Temp:  2250
    //   Outdoor Temp: 1800
    snprintf(_status.name, sizeof(DaikinSerialApi::daikin_status_info_t::name), "%s", "DaikinSerialLocalMock");
    _status.power = DaikinEnums::Power::POWER_ON;
    _status.mode = DaikinEnums::Mode::MODE_HEATING;
    _status.fan_rate = DaikinEnums::FanRate::FAN_LEVEL5;
    _status.fan_dir = DaikinEnums::FanDir::FanDir_OFF;
    _status.preset = DaikinEnums::Preset::PRESET_NONE;
    _status.target_temp = 2200;
    _status.indoor_temp = 2200;
    _status.outdoor_temp = 3100;

  }

  virtual ~DaikinSerialLocalMock() {}

  // Initialise the underlying SerialTransfer instance.
  // Call this from setup() after the HardwareSerial port has been started.
  void begin(HardwareSerial& port) {
    tempSimulationUpdateTimer.setTimeOutTime(SIMULATION_UPDATE_INTERVAL);
    tempSimulationUpdateTimer.reset();
  }

  // -------------------------------------------------------------------------
  // DaikinSerialApi overrides
  // -------------------------------------------------------------------------

  ApiResult getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override {
    *addr = _ipaddr;
    return ApiResult::API_RESULT_OK;
  }

  ApiResult setTargetTemperature(int16_t temperature, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override {
    _status.target_temp = temperature;
    return ApiResult::API_RESULT_OK;
  }

  ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override {
    *status = _status;
    return ApiResult::API_RESULT_OK;
  }

  // 
  void loop() {
    simulateTemperature();
  }

private:

  /**
   * @brief Simulate local temperature changes.
   * The local temperature must drifts towards an hypothetical "target_temp".
   * When heating, target_temp is the heating setpoint.
   * When not heating, target_temp is the default room temperature.
   */
  void simulateTemperature() {
    // Make sure we do not call this function too often...
    if (tempSimulationUpdateTimer.getTimeOutTime() != 0 && !tempSimulationUpdateTimer.hasTimedOut()) {
      return; // too soon
    }
    // reset timer for next iteration timestamps
    tempSimulationUpdateTimer.reset();

    // Get actuals values
    int16_t local_temp = _status.indoor_temp;
    int16_t setpoint = _status.target_temp;
    uint8_t compressor_freq = _status.compressor_freq;

    int16_t new_local_temp = local_temp;
    int16_t target_temp = setpoint;
    
    // Compute target temperature
    if (compressor_freq == SIMULATION_COMPRESSOR_FREQ_IDLE) {
      // When off, temperature drifts toward room temp
      target_temp = SIMULATION_DEFAULT_ROOM_TEMPERATURE; // SIMULATION_DEFAULT_ROOM_TEMPERATURE is the new setpoint
    }

    // Update new_temp towards target temperature
    if (local_temp < target_temp - SIMULATION_TEMPERATURE_DIFF_HIGH ) {
      new_local_temp += SIMULATION_TEMPERATURE_STEP_HIGH;   // Heat faster when far from setpoint
    } else if (local_temp < target_temp) {
      new_local_temp += SIMULATION_TEMPERATURE_STEP_LOW;    // Heat slower when close to setpoint
    } else if (local_temp > target_temp + SIMULATION_TEMPERATURE_DIFF_HIGH) {
      new_local_temp -= SIMULATION_TEMPERATURE_STEP_HIGH;   // Cool faster when too hot
    } else if (local_temp > target_temp) {
      new_local_temp -= SIMULATION_TEMPERATURE_STEP_LOW;    // Cool slower when close
    }
    
    // Update official local temperature if it needs to change
    if (new_local_temp != local_temp) {
      _status.indoor_temp = new_local_temp;
    }

    log_i("Simulation Update --> Temp: %.1f°C --> %.1f°C, Setpoint: %.1f°C, TargetTemp: %.1f°C",
                  local_temp / 100.0,
                  new_local_temp / 100.0,
                  setpoint / 100.0,
                  target_temp / 100.0);
  }

};
