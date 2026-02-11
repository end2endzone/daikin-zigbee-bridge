// EnergyCalculator.h
// No Zigbee dependencies

#pragma once

#include <SoftTimers.h>

#define STELPRO_MAX_POWER_WATTS 4000  // Maximum power for baseboard heater

class EnergyCalculator {
private:
  // Temperature values (in hundredths of degrees Celsius)
  int16_t _local_temperature;
  int16_t _heating_setpoint;
  
  // System state
  bool _system_on;  // true = heating mode, false = off mode
  
  // Calculated values
  uint8_t _pi_heating_demand;      // 0-100%
  int32_t _instantaneous_demand;    // Power in watts
  uint64_t _current_summation;      // Total energy in Wh
  
  // Timing
  SoftTimer _computation_timer;
  unsigned long _refresh_time;  // Time between energy computations (ms)
  
  // Constants
  static constexpr int16_t TEMP_THRESHOLD_HIGH = 200;  // 2°C in hundredths

  // Private computation method
  void computeEnergy() {
    // Calculate heating demand based on temperature difference
    if (_system_on) {
      int16_t temp_diff = _heating_setpoint - _local_temperature;
      
      if (temp_diff > TEMP_THRESHOLD_HIGH) {
        // More than 2°C below setpoint - full demand
        _pi_heating_demand = 100;
      } else if (temp_diff > 0) {
        // 0-2°C below setpoint - proportional demand
        _pi_heating_demand = (temp_diff * 100) / TEMP_THRESHOLD_HIGH;
      } else {
        // At or above setpoint - no demand
        _pi_heating_demand = 0;
      }
    } else {
      _pi_heating_demand = 0;
    }
    
    // Calculate instantaneous power demand
    _instantaneous_demand = (STELPRO_MAX_POWER_WATTS * _pi_heating_demand) / 100;
    
    // Calculate energy consumed since last computation
    unsigned long elapsed = _computation_timer.getElapsedTime();
    if (elapsed > 0) {
      double elapsed_hours = elapsed / 3600000.0;
      double energy_wh = _instantaneous_demand * elapsed_hours;
      _current_summation += static_cast<uint64_t>(energy_wh);
    }
  }

public:
  EnergyCalculator() :
    _local_temperature(2100),  // 21.0°C
    _heating_setpoint(2000),   // 20.0°C
    _system_on(true),
    _pi_heating_demand(0),
    _instantaneous_demand(0),
    _current_summation(0),
    _refresh_time(5 * 60 * 1000) {  // 5 minutes default
    
    _computation_timer.setTimeOutTime(_refresh_time);
    _computation_timer.reset();
  }
  
  // Configuration setters
  void setRefreshTime(unsigned long refresh_time_ms) {
    _refresh_time = refresh_time_ms;
    // Trigger immediate computation with new timing
    computeEnergy();
    _computation_timer.setTimeOutTime(_refresh_time);
    _computation_timer.reset();
  }
  
  void setLocalTemperature(int16_t temperature) {
    _local_temperature = temperature;
  }
  
  void setHeatingSetpoint(int16_t setpoint) {
    _heating_setpoint = setpoint;
  }
  
  void setSystemMode(bool system_on) {
    _system_on = system_on;
    if (!system_on) {
      // When system is off, demand is zero
      _pi_heating_demand = 0;
      _instantaneous_demand = 0;
    }
  }
  
  // Getters
  uint8_t getPIHeatingDemand() const {
    return _pi_heating_demand;
  }
  
  int32_t getInstantaneousDemandWatts() const {
    return _instantaneous_demand;
  }
  
  double getInstantaneousDemandKilowatts() const {
    return _instantaneous_demand / 1000.0;
  }
  
  uint64_t getCurrentSummationDelivered() const {
    return _current_summation;
  }
  
  uint16_t getRunningState() const {
    // 0x0000 = IDLE, 0x0001 = HEAT
    return (_pi_heating_demand > 0) ? 0x0001 : 0x0000;
  }
  
  // Main update function - call this in loop()
  void loop() {
    if (_computation_timer.hasTimedOut()) {
      computeEnergy();
      _computation_timer.reset();  // Reset for next interval
    }
  }
  
  // Reset energy accumulation
  void reset() {
    _current_summation = 0;
    _computation_timer.reset();
  }
};
