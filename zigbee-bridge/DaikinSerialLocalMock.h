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
    //   Comp Freq:    0
    snprintf(_status.name, sizeof(DaikinSerialApi::daikin_status_info_t::name), "%s", "DaikinSerialLocalMock");
    _status.power = DaikinEnums::Power::POWER_ON;
    _status.mode = DaikinEnums::Mode::MODE_HEATING;
    _status.fan_rate = DaikinEnums::FanRate::FAN_LEVEL5;
    _status.fan_dir = DaikinEnums::FanDir::FanDir_OFF;
    _status.preset = DaikinEnums::Preset::PRESET_NONE;
    _status.target_temp = SIMULATION_DEFAULT_HEATING_SETPOINT;
    _status.indoor_temp = SIMULATION_DEFAULT_ROOM_TEMPERATURE;
    _status.outdoor_temp = 3100;
    _status.compressor_freq = 0;
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
   * Makes the local temperature drift toward an hypothetical "setpoint".
   */
  void simulateTemperature() {
    // Make sure we do not call this function too often...
    if (tempSimulationUpdateTimer.getTimeOutTime() != 0 && !tempSimulationUpdateTimer.hasTimedOut()) {
      return; // too soon
    }
    // reset timer for next iteration timestamps
    tempSimulationUpdateTimer.reset();

    // Get actuals values
    const int16_t old_local_temp = _status.indoor_temp;
    const int16_t setpoint = _status.target_temp;
    const uint8_t old_compressor_freq = _status.compressor_freq;
    const DaikinEnums::Mode old_mode = _status.mode;

    int16_t new_local_temp = old_local_temp;
    uint8_t new_compressor_freq = old_compressor_freq;
    DaikinEnums::Mode new_mode = old_mode;
    
    // Update new_local_temp towards target temperature or room temperature
    if (old_local_temp < setpoint) {
      new_mode = DaikinEnums::Mode::MODE_HEATING;

      // Calculate speed gradient based on current temperature and target temperature.
      int16_t diff = abs(old_local_temp - setpoint);
      if (diff > SIMULATION_TEMPERATURE_DIFF_HIGH) {
        new_local_temp += SIMULATION_TEMPERATURE_STEP_HIGH;   // Heat faster when far from setpoint
        new_compressor_freq = SIMULATION_COMPRESSOR_FREQ_HIGH;
      } else {
        new_local_temp += SIMULATION_TEMPERATURE_STEP_LOW;    // Heat slower when close to setpoint
        new_compressor_freq = SIMULATION_COMPRESSOR_FREQ_LOW;
      }
    } else {
      new_mode = DaikinEnums::Mode::MODE_FAN;
      new_compressor_freq = SIMULATION_COMPRESSOR_FREQ_IDLE;

      // When not heating, local temperature drifts towards the default room temperature,
      // but only if local temperature > room temperature.
      if (old_local_temp > setpoint) {

        // Calculate speed gradient based on current temperature and target temperature.
        int16_t diff = abs(old_local_temp - setpoint);
        if (diff > SIMULATION_TEMPERATURE_DIFF_HIGH) {
          new_local_temp -= SIMULATION_TEMPERATURE_STEP_HIGH;   // Cool faster when too hot
        } else {
          new_local_temp -= SIMULATION_TEMPERATURE_STEP_LOW;    // Cool slower when close
        }
      }
    }

    bool has_changed = false;

    // Update official local temperature if it needs to change
    if (new_local_temp != old_local_temp) {
      _status.indoor_temp = new_local_temp;
      has_changed = true;
    }

    // Update official mode if it needs to change
    if (new_mode != old_mode) {
      _status.mode = new_mode;
      has_changed = true;
    }

    // Update official compressor frequency if it needs to change
    if (new_compressor_freq != old_compressor_freq) {
      _status.compressor_freq = new_compressor_freq;
      has_changed = true;
    }

    if (has_changed) {
      log_i("Simulation update: Temp: %.1f°C --> %.1f°C, Setpoint: %.1f°C, Comp. Freq: %d Hz --> %d Hz",
        old_local_temp / 100.0,
        new_local_temp / 100.0,
        setpoint / 100.0,
        old_compressor_freq,
        new_compressor_freq);
    }
    // else {
    //  log_i("Simulation Update --> No change.");
    //}
  }

};
