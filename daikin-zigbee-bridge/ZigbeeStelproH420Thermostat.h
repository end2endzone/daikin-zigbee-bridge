/**
 * @brief Zigbee Stelpro H420/HT402 Thermostat endpoint
 * 
 * Emulates a Stelpro HT402 (Hilo) line-voltage thermostat for Zigbee2MQTT.
 * This is a heating-only baseboard heater thermostat (4000W @ 240V).
 */

#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "ZigbeeEP.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_thermostat.h"
#include "zcl/esp_zigbee_zcl_thermostat_ui_config.h"
#include "esp_zigbee_type.h"

#ifdef USE_ENERGY_CALCULATOR
#include "EnergyCalculator.h"
#endif // #ifdef USE_ENERGY_CALCULATOR

// Stelpro HT402 specific configuration
#define STELPRO_ENDPOINT 25               // HT402 uses endpoint 25, not 10!
#define STELPRO_MANUFACTURER_CODE 0x1297  // 4759 in decimal
#define STELPRO_MANUFACTURER_NAME "Stelpro"
#define STELPRO_MODEL_NAME "HT402"

// Default Stelpro thermostat values (in hundredths of degrees Celsius)
#define STELPRO_DEFAULT_TEMPERATURE           2100  // 22.0°C
#define STELPRO_DEFAULT_HEATING_SETPOINT      2000  // 20.0°C
#define STELPRO_MIN_HEAT_SETPOINT              500  //  5.0°C
#define STELPRO_MAX_HEAT_SETPOINT             3000  // 30.0°C

// HT402 only supports HEAT mode
#define THERMOSTAT_SYSTEM_MODE_OFF 0x00
#define THERMOSTAT_SYSTEM_MODE_HEAT 0x04

// Running state values
#define THERMOSTAT_RUNNING_STATE_IDLE 0x0000
#define THERMOSTAT_RUNNING_STATE_HEAT 0x0001

// Custom Stelpro custom attributes ID
//
// Stelpro outdoor temperature:
//    The outdoor temperature seems to be exposed as a custom attribute.
//    The code in https://github.com/Koenkk/zigbee-herdsman-converters, seems to refer to
//    strings such as `stelpro_thermostat_outdoor_temperature` and `StelproOutdoorTemp`.
//    There is already a known attribute for outdoor temperature in the specification according to:
//    https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32/api-reference/zcl/esp_zigbee_zcl_thermostat.html
//    Attribute `ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID` might be the actual measured temperature by a physical sensor
//    on the device and not an attribute whose purpose is to "display a value on the thermostat".
//    Do not expose this attribute for the moment.
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID 0xDEADBEEF // (maybe 0x4001)
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

// Power & Energy:
//    According to https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/devices/stelpro.ts, lines 10 to 29,
//    Stelpro seems to expose power and energy in the Thermostat cluster using custom attributes.
//    Power  as custom attribute (16392, 0x4008) and
//    Energy as custom attribute (16393, 0x4009).
//    Do not expose this attribute for the moment.
#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_POWER_ID 0x4008
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_ENERGY_ID 0x4009
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

// Maximum power for baseboard heater
#define STELPRO_MAX_POWER_WATTS 4000

/**
 * @brief Zigbee Stelpro H420 thermostat device clusters configuration
 * See `esp_zigbee_type.h` in `esp-zigbee-sdk` for details of each types at:
 * https://github.com/espressif/esp-zigbee-sdk/blob/main/components/esp-zigbee-lib/include/esp_zigbee_type.h
 *
 * Exposed clusters for Zigbee Stelpro H420 thermostat according to zigbee-herdsman-converters (Zigbee2MQTT) :
 * https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/devices/stelpro.ts#L31
 * Presets:
 *   https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/lib/exposes.ts#L950
 */
 ;
typedef struct {
  esp_zb_basic_cluster_cfg_t basic_cfg;                                 /*!<  Basic cluster configuration, @ref esp_zb_basic_cluster_cfg_s */
  esp_zb_identify_cluster_cfg_t identify_cfg;                           /*!<  Identify cluster configuration, @ref esp_zb_identify_cluster_cfg_s */
  esp_zb_groups_cluster_cfg_t groups_cfg;                               /*!<  Groups cluster configuration, @ref esp_zb_groups_cluster_cfg_s */
  esp_zb_thermostat_cluster_cfg_t thermostat_cfg;                       /*!<  Thermostat cluster configuration, @ref esp_zb_thermostat_cluster_cfg_t */
  esp_zb_thermostat_ui_config_cluster_cfg_t thermostat_ui_config_cfg;   /*!<  Thermostat cluster configuration, @ref esp_zb_thermostat_ui_config_cluster_cfg_s */
  //esp_zb_temperature_meas_cluster_cfg_t temp_meas_cfg;                /*!<  Temperature measurement cluster configuration, @ref esp_zb_temperature_meas_cluster_cfg_s */
} zigbee_stelpro_thermostat_cfg_t;

/**
 * @brief Default configuration for Stelpro H420 thermostat
 */
#define ZIGBEE_DEFAULT_STELPRO_THERMOSTAT_CONFIG()                                                    \
  {                                                                                                   \
    .basic_cfg = {                                                                                    \
      .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,                                      \
      .power_source = ESP_ZB_AF_NODE_POWER_SOURCE_CONSTANT_POWER,                                     \
    },                                                                                                \
    .identify_cfg = {                                                                                 \
      .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,                               \
    },                                                                                                \
    .groups_cfg = {                                                                                   \
      .groups_name_support_id = ESP_ZB_ZCL_GROUPS_NAME_SUPPORT_DEFAULT_VALUE,                         \
    },                                                                                                \
    .thermostat_cfg = {                                                                               \
      .local_temperature = ESP_ZB_ZCL_THERMOSTAT_LOCAL_TEMPERATURE_DEFAULT_VALUE,                     \
      .occupied_cooling_setpoint = ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE,     \
      .occupied_heating_setpoint = ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE,     \
      .control_sequence_of_operation = ESP_ZB_ZCL_THERMOSTAT_CONTROL_SEQ_OF_OPERATION_DEFAULT_VALUE,  \
      .system_mode = ESP_ZB_ZCL_THERMOSTAT_CONTROL_SYSTEM_MODE_DEFAULT_VALUE,                         \
    },                                                                                                \
    .thermostat_ui_config_cfg = {                                                                     \
      .temperature_display_mode = ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE, \
      .keypad_lockout = ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_DEFAULT_VALUE,                 \
    },                                                                                                \
  }

class ZigbeeStelproH420Thermostat : public ZigbeeEP {
public:
  ZigbeeStelproH420Thermostat(uint8_t endpoint = STELPRO_ENDPOINT);
  ~ZigbeeStelproH420Thermostat() {}

  // Callback setters
  void onTemperatureChange(void (*callback)(int16_t)) {
    _on_temperature_change = callback;
  }
  void onOutdoorTemperatureChange(void (*callback)(int16_t)) {
    _on_outdoor_temperature_change = callback;
  }
  void onSetpointChange(void (*callback)(int16_t)) {
    _on_setpoint_change = callback;
  }
  void onSystemModeChange(void (*callback)(uint8_t)) {
    _on_system_mode_change = callback;
  }
  void onPIHeatingDemandChange(void (*callback)(uint8_t)) {
    _on_pi_heating_demand_change = callback;
  }
  void onRunningStateChange(void (*callback)(uint16_t)) {
    _on_running_state_change = callback;
  }
  void onDisplayModeChange(void (*callback)(uint8_t)) {
    _on_display_mode_change = callback;
  }
  void onKeypadLockoutChange(void (*callback)(uint8_t)) {
    _on_keypad_lockout_change = callback;
  }

  // Getters for current values
  int16_t getLocalTemperature() const { return _local_temperature; }
  int16_t getHeatingSetpoint() const { return _heating_setpoint; }
  uint8_t getSystemMode() const { return _system_mode; }
  uint8_t getPIHeatingDemand() const { return _pi_heating_demand; }
  uint16_t getRunningState() const { return _running_state; }
  int16_t getOutdoorTemperature() const { return _outdoor_temperature; }
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  int16_t getStelproOutdoorTemp() const { return _stelpro_outdoor_temp; }
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

  // Setters (update Zigbee attributes)
  bool setLocalTemperature(int16_t temperature);
  bool setHeatingSetpoint(int16_t setpoint);
  bool setSystemMode(uint8_t mode);
  bool setPIHeatingDemand(uint8_t demand);
  bool setRunningState(uint16_t state);
  bool setOutdoorTemperature(int16_t temperature);
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  bool setStelproOutdoorTemp(int16_t temperature);
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Power reporting setters
  bool setInstantaneousDemand(int32_t demand_watts);
  bool setCurrentSummationDelivered(uint64_t energy_wh);
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

  // Update energy calculations (call in loop)
  void updateEnergy();

  void debugClusterList();
  
private:
  void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;
  
  // Energy calculator
#ifdef USE_ENERGY_CALCULATOR
  EnergyCalculator _energy_calc;
#endif // #ifdef USE_ENERGY_CALCULATOR
  
  /**
   * @brief Create Stelpro H420 thermostat cluster list
   * 
   * Contains:
   * - Basic cluster (with manufacturer name and model)
   * - Identify cluster
   * - Thermostat cluster (with all attributes including custom 0x4008 and 0x4009)
   * - Thermostat UI Config cluster
   */
  esp_zb_cluster_list_t *zigbee_stelpro_thermostat_clusters_create(zigbee_stelpro_thermostat_cfg_t *thermostat_cfg);

  // Callback functions
  void (*_on_temperature_change)(int16_t);
  void (*_on_outdoor_temperature_change)(int16_t);
  void (*_on_setpoint_change)(int16_t);
  void (*_on_system_mode_change)(uint8_t);
  void (*_on_pi_heating_demand_change)(uint8_t);
  void (*_on_running_state_change)(uint16_t);
  void (*_on_display_mode_change)(uint8_t);
  void (*_on_keypad_lockout_change)(uint8_t);

  // Thermostat state variables
  int16_t _local_temperature;
  int16_t _heating_setpoint;
  int16_t _outdoor_temperature;
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  int16_t _stelpro_outdoor_temp;
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  uint8_t _system_mode;
  uint8_t _pi_heating_demand;
  uint16_t _running_state;
  uint8_t _temperature_display_mode; // type ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM
  uint8_t _keypad_lockout;
  zb_uint8_t _occupancy;
  uint8_t _display_mode;
  int16_t _min_heat_setpoint;
  int16_t _max_heat_setpoint;

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Metering cluster variables
  esp_zb_int24_t _instantaneous_demand;
  esp_zb_uint48_t _current_summation_delivered;
  esp_zb_uint24_t _multiplier;
  esp_zb_uint24_t _divisor;
  uint8_t _unit_of_measure;
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

};
