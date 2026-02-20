/**
 * @brief Zigbee Stelpro H420/HT402 Thermostat endpoint
 * 
 * Emulates a Stelpro HT402 (Hilo) line-voltage thermostat for Zigbee2MQTT.
 * This is a heating-only baseboard heater thermostat (4000W @ 240V).
 */

#pragma once

// FORCE USAGE OF ZIGBEEATTRIBUTE CLASSES
#ifndef USE_ZB_CLASSES
#define USE_ZB_CLASSES
#endif

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "ZigbeeEP.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_thermostat.h"
#include "zcl/esp_zigbee_zcl_thermostat_ui_config.h"
#include "esp_zigbee_type.h"
#include "logging.h"
#ifdef USE_ZB_CLASSES
#include "ZigbeeAttributeT.hpp"
#else // USE_ZB_CLASSES
#endif // USE_ZB_CLASSES


#ifdef USE_ENERGY_CALCULATOR
#include "EnergyCalculator.h"
#endif // #ifdef USE_ENERGY_CALCULATOR

// Stelpro HT402 specific configuration
// Notes:
//   HT402 uses endpoint 25, not 10!
#define STELPRO_ENDPOINT 25
#define STELPRO_MANUFACTURER_CODE 0x1297  // 4759 in decimal
#define STELPRO_MANUFACTURER_NAME "Stelpro"
#define STELPRO_MODEL_NAME "HT402"

// Default Stelpro thermostat values (in hundredths of degrees Celsius)
#define STELPRO_DEFAULT_TEMPERATURE           2200  // 22.0°C
#define STELPRO_DEFAULT_HEATING_SETPOINT      2000  // 20.0°C
#define STELPRO_MIN_HEAT_SETPOINT              500  //  5.0°C
#define STELPRO_MAX_HEAT_SETPOINT             3000  // 30.0°C
#define STELPRO_TEMP_MEASUREMENT_TOLERANCE      50  //  0.5°C
// To allow `occupied_cooling_setpoint` with values from 5.0°C to 30.0°C, 
// we need to set the following attribute values:
#define STELPRO_MIN_COOL_SETPOINT              500  //  5.0°C
#define STELPRO_MAX_COOL_SETPOINT             (500 + STELPRO_MAX_HEAT_SETPOINT) // make sure value is greater than STELPRO_MAX_HEAT_SETPOINT
#define STELPRO_OCCUPIED_COOLING_SETPOINT     (500 + STELPRO_MAX_HEAT_SETPOINT) // make sure value is greater than STELPRO_MAX_HEAT_SETPOINT


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
 * Notes:
 *   - For `occupied_cooling_setpoint`, do not use default value `ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE` of 26.0°C.
 *     If you do, this will set an upper limit for `occupied_heating_setpoint` at 25.0°C.
 *     Controllers that try to set a value above 25.0°C will get a "Invalid Value" error such as:
 *     ```
 *     z2m: Publish 'set' 'occupied_heating_setpoint' to '0x404ccafffe563248' failed:
 *     'Error: ZCL command 0x404ccafffe563248/25 hvacThermostat.write(
 *     {"occupiedHeatingSetpoint":2900}, 
 *     {"timeout":10000,
 *       "disableResponse":false,
 *       "disableRecovery":false,
 *       "disableDefaultResponse":true,
 *       "direction":0,
 *       "reservedBits":0,
 *       "writeUndiv":false})
 *       failed (Status 'INVALID_VALUE')'
 *     ```
 *     I think the zigbee stack can not allow a situation where `occupied_heating_setpoint > occupied_cooling_setpoint`.
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
      .occupied_cooling_setpoint = STELPRO_OCCUPIED_COOLING_SETPOINT,                                 \
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
  // Thermostat cluster
  void onLocalTemperatureChange(void (*callback)(int16_t)) {
    callbacks._on_local_temperature_change = callback;
  }
  void onOccupiedCoolSetpointChange(void (*callback)(int16_t)) {
    callbacks._on_occupied_cool_setpoint_change = callback;
  }
  void onOccupiedHeatSetpointChange(void (*callback)(int16_t)) {
    callbacks._on_occupied_heat_setpoint_change = callback;
  }
  void onControlSequenceOfOperationChange(void (*callback)(uint8_t)) {
    callbacks._on_control_sequence_of_operation_change = callback;
  }
  void onSystemModeChange(void (*callback)(uint8_t)) {
    callbacks._on_system_mode_change = callback;
  }
  // Thermostat UI cluster
  void onDisplayModeChange(void (*callback)(uint8_t)) {
    callbacks._on_display_mode_change = callback;
  }
  void onKeypadLockoutChange(void (*callback)(uint8_t)) {
    callbacks._on_keypad_lockout_change = callback;
  }
  // Thermostat cluster, additional attributes
  void onRunningStateChange(void (*callback)(uint16_t)) {
    callbacks._on_running_state_change = callback;
  }
  void onPIHeatingDemandChange(void (*callback)(uint8_t)) {
    callbacks._on_pi_heating_demand_change = callback;
  }
  void onOutdoorTemperatureChange(void (*callback)(int16_t)) {
    callbacks._on_outdoor_temperature_change = callback;
  }
  void onOccupancyChange(void (*callback)(zb_uint8_t)) {
    callbacks._on_occupancy_change = callback;
  } 

#ifdef USE_ZB_CLASSES
  // Zigbee atributes getters
  // Thermostat cluster mandatory attributes
  bool getLocalTemperature(int16_t& output) const               { return _local_temperature             .get(output); }
  bool getOccupiedCoolingSetpoint(int16_t& output) const        { return _occupied_cooling_setpoint     .get(output); }
  bool getOccupiedHeatingSetpoint(int16_t& output) const        { return _occupied_heating_setpoint     .get(output); }
  bool getControlSequenceOfOperation(uint8_t& output) const     { return _control_sequence_of_operation .get(output); }
  bool getSystemMode(uint8_t& output) const                     { return _system_mode                   .get(output); }
  // Thermostat cluster additional attributes
  bool getRunningState(uint16_t& output) const                  { return _running_state                 .get(output); }
  bool getPIHeatingDemand(uint8_t& output) const                { return _pi_heating_demand             .get(output); }
  bool getOutdoorTemperature(int16_t& output) const             { return _outdoor_temperature           .get(output); }
  bool getOccupancy(zb_uint8_t& output) const                   { return _occupancy                     .get(output); }
  bool getMinHeatingSetpointLimit(int16_t& output) const        { return _min_heat_setpoint_limit       .get(output); }
  bool getMaxHeatingSetpointLimit(int16_t& output) const        { return _max_heat_setpoint_limit       .get(output); }
  bool getAbsMinHeatingSetpointLimit(int16_t& output) const     { return _abs_min_heat_setpoint_limit   .get(output); }
  bool getAbsMaxHeatingSetpointLimit(int16_t& output) const     { return _abs_max_heat_setpoint_limit   .get(output); }
  bool getMinCoolingSetpointLimit(int16_t& output) const        { return _min_cool_setpoint_limit       .get(output); }
  bool getMaxCoolingSetpointLimit(int16_t& output) const        { return _max_cool_setpoint_limit       .get(output); }
  bool getAbsMinCoolingSetpointLimit(int16_t& output) const     { return _abs_min_cool_setpoint_limit   .get(output); }
  bool getAbsMaxCoolingSetpointLimit(int16_t& output) const     { return _abs_max_cool_setpoint_limit   .get(output); }
  // Thermostat UI cluster
  bool getTemperatureDisplayMode(uint8_t& output) const         { return _ui_config_display_mode        .get(output); }
  bool getKeypadLockout(uint8_t& output) const                  { return _ui_config_keypad_lockout      .get(output); }
#else // USE_ZB_CLASSES
  // Zigbee atributes getters
  // Thermostat cluster
  bool getLocalTemperature(int16_t& output) const               { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID,                         output); }
  bool getOccupiedCoolingSetpoint(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID,                 output); }
  bool getOccupiedHeatingSetpoint(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID,                 output); }
  bool getControlSequenceOfOperation(uint8_t& output) const     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID,                               output); }
  bool getSystemMode(uint8_t& output) const                     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID,             output); }
  // Thermostat UI cluster
  bool getTemperatureDisplayMode(uint8_t& output) const         { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID,        output); }
  bool getKeypadLockout(uint8_t& output) const                  { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID,                  output); }
  // Thermostat cluster, additional attributes
  bool getRunningState(uint16_t& output) const                  { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID,                  output); }
  bool getPIHeatingDemand(uint8_t& output) const                { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID,                         output); }
  bool getOutdoorTemperature(int16_t& output) const             { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID,                       output); }
  bool getOccupancy(zb_uint8_t& output) const                   { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID,                                 output); }
  // Heating setpoints limits
  bool getMinHeatingSetpointLimit(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID,                   output); }
  bool getMaxHeatingSetpointLimit(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID,                   output); }
  bool getAbsMinHeatingSetpointLimit(int16_t& output) const     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID,               output); }
  bool getAbsMaxHeatingSetpointLimit(int16_t& output) const     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID,               output); }
  // Cooling setpoints limits
  bool getMinCoolingSetpointLimit(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID,                   output); }
  bool getMaxCoolingSetpointLimit(int16_t& output) const        { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID,                   output); }
  bool getAbsMinCoolingSetpointLimit(int16_t& output) const     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID,               output); }
  bool getAbsMaxCoolingSetpointLimit(int16_t& output) const     { return getGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID,               output); }
#endif // USE_ZB_CLASSES

#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  int16_t getStelproOutdoorTemp() const { return _stelpro_outdoor_temp; }
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

  // Zigbee atributes setters
  // Thermostat cluster mandatory attributes
  bool setLocalTemperature(int16_t temperature);
  bool setOccupiedCoolingSetpoint(int16_t setpoint);
  bool setOccupiedHeatingSetpoint(int16_t setpoint);
  bool setControlSequenceOfOperation(uint8_t csop);
  bool setSystemMode(uint8_t mode);
  // Thermostat cluster additional attributes
  bool setRunningState(uint16_t running);
  bool setPIHeatingDemand(uint8_t demand);
  bool setOutdoorTemperature(int16_t temperature);
  bool setOccupancy(zb_uint8_t occupancy);
  //bool setMinHeatingSetpointLimit(int16_t value);
  //bool setMaxHeatingSetpointLimit(int16_t value);
  //bool setAbsMinHeatingSetpointLimit(int16_t value);
  //bool setAbsMaxHeatingSetpointLimit(int16_t value);
  //bool setMinCoolingSetpointLimit(int16_t value);
  //bool setMaxCoolingSetpointLimit(int16_t value);
  //bool setAbsMinCoolingSetpointLimit(int16_t value);
  //bool setAbsMaxCoolingSetpointLimit(int16_t value);
  // Thermostat UI cluster
  bool setTemperatureDisplayMode(uint8_t temperature);
  bool setKeypadLockout(uint8_t lockout);

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

  bool setup();

#ifdef USE_ZB_CLASSES
  void printZigbeeAttributes();
#else // USE_ZB_CLASSES
private:
  bool getGenericAttribute(uint16_t cluster_id, uint16_t attr_id, void* output_ptr, size_t output_size) const;
  bool setGenericAttribute(uint16_t cluster_id, uint16_t attr_id, const void* value_ptr, size_t value_size) const;
public:
  template<typename T> inline bool getGenericAttribute(uint16_t cluster_id, uint16_t attr_id, T& output) const { return getGenericAttribute(cluster_id, attr_id, &output, sizeof(T)); }
  template<typename T> inline bool setGenericAttribute(uint16_t cluster_id, uint16_t attr_id, T& output) const { return setGenericAttribute(cluster_id, attr_id, &output, sizeof(T)); }
#endif // USE_ZB_CLASSES

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
  typedef struct zb_zcl_thermostat_callbacks_s {
    // Thermostat cluster
    void (*_on_local_temperature_change)(int16_t);
    void (*_on_occupied_cool_setpoint_change)(int16_t);
    void (*_on_occupied_heat_setpoint_change)(int16_t);
    void (*_on_control_sequence_of_operation_change)(uint8_t);
    void (*_on_system_mode_change)(uint8_t);
    // Thermostat UI cluster
    void (*_on_display_mode_change)(uint8_t);
    void (*_on_keypad_lockout_change)(uint8_t);
    // Thermostat cluster, additional attributes
    void (*_on_running_state_change)(uint16_t);
    void (*_on_pi_heating_demand_change)(uint8_t);
    void (*_on_outdoor_temperature_change)(int16_t);
    void (*_on_occupancy_change)(zb_uint8_t);
  } zb_zcl_thermostat_callbacks_t;
  zb_zcl_thermostat_callbacks_t callbacks = {0};

  // Thermostat state variables
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  int16_t _stelpro_outdoor_temp;
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  
#ifdef USE_ZB_CLASSES
  // Thermostat cluster mandatory attributes
  ZigbeeAttribute<int16_t>    _local_temperature                ;
  ZigbeeAttribute<int16_t>    _occupied_cooling_setpoint        ;
  ZigbeeAttribute<int16_t>    _occupied_heating_setpoint        ;
  ZigbeeAttribute<uint8_t>    _control_sequence_of_operation    ;
  ZigbeeAttribute<uint8_t>    _system_mode                      ;
  // Thermostat cluster additional attributes
  ZigbeeAttribute<uint16_t>   _running_state                    ;
  ZigbeeAttribute<uint8_t>    _pi_heating_demand                ;
  ZigbeeAttribute<int16_t>    _outdoor_temperature              ;
  ZigbeeAttribute<zb_uint8_t> _occupancy                        ;
  ZigbeeAttribute<int16_t>    _min_heat_setpoint_limit          ;
  ZigbeeAttribute<int16_t>    _max_heat_setpoint_limit          ;
  ZigbeeAttribute<int16_t>    _abs_min_heat_setpoint_limit      ;
  ZigbeeAttribute<int16_t>    _abs_max_heat_setpoint_limit      ;
  ZigbeeAttribute<int16_t>    _min_cool_setpoint_limit          ;
  ZigbeeAttribute<int16_t>    _max_cool_setpoint_limit          ;
  ZigbeeAttribute<int16_t>    _abs_min_cool_setpoint_limit      ;
  ZigbeeAttribute<int16_t>    _abs_max_cool_setpoint_limit      ;
  // Thermostat UI cluster mandatory attributes
  ZigbeeAttribute<uint8_t>    _ui_config_display_mode           ;
  ZigbeeAttribute<uint8_t>    _ui_config_keypad_lockout         ;

#else // USE_ZB_CLASSES
  uint16_t _running_state;
  uint8_t _pi_heating_demand;
  int16_t _outdoor_temperature;
  zb_uint8_t _occupancy;
#endif // USE_ZB_CLASSES

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Metering cluster variables
  esp_zb_int24_t _instantaneous_demand;
  esp_zb_uint48_t _current_summation_delivered;
  esp_zb_uint24_t _multiplier;
  esp_zb_uint24_t _divisor;
  uint8_t _unit_of_measure;
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

};
