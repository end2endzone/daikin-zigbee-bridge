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
#include "logging.h"
#include "ZigbeeAttributeT.hpp"


#ifdef USE_ENERGY_CALCULATOR
#include "EnergyCalculator.h"
#endif // #ifdef USE_ENERGY_CALCULATOR

// Stelpro HT402 specific configuration
// Notes:
//   HT402 uses endpoint 25, not 10!
#define STELPRO_ENDPOINT 25
#define STELPRO_MANUFACTURER_CODE 0x1185
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

// Running state values (each bit is the state of something)
#define THERMOSTAT_RUNNING_STATE_IDLE 0x0000
#define THERMOSTAT_RUNNING_STATE_HEAT (THERMOSTAT_RUNNING_STATE_IDLE | ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT)

// Stelpro outdoor temperature:
//  The displayed outdoor temperature is exposed as a custom attribute.
//  The code in https://github.com/Koenkk/zigbee-herdsman-converters, to refer to
//  strings such as `stelpro_thermostat_outdoor_temperature` and `StelproOutdoorTemp`.
//  The Zigbee specification has already a known attribute for outdoor temperature in the specification according to:
//  https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32/api-reference/zcl/esp_zigbee_zcl_thermostat.html
//  Attribute `ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID` is not related to this attribute. It refers to the actual measured temperature
//  by a physical sensor on the device and not an attribute whose purpose is to "display a value on the thermostat".
//  Setting this attribute to value 12 using zigbee2mqtt outputs the following error:
//    ```
//    [2/25/2026, 10:40:20 PM] z2m: Publish 'set' 'write' to '0x232cabcdef123456' failed: 'Error: ZCL command 0x232cabcdef123456/25 hvacThermostat.write({"StelproOutdoorTemp":12},
//    {"timeout":10000,"disableResponse":false,"disableRecovery":false,"disableDefaultResponse":true,"direction":0,"reservedBits":0,"writeUndiv":false}) failed
//    (Delivery failed for '14298'.)'
//    ```
//    where 14298 is 0x37DA.
//  Custom attributes should be greater than 0x4000.
//  The `StelproOutdoorTemp` attribute definition is available at 
//    * https://github.com/Koenkk/zigbee-specification/blob/3c96049cd632d0780b9f79ebc8aecc672d4c4505/src/zcl/definition/clusters.ts#L1906
//    * https://github.com/Koenkk/zigbee-herdsman/blob/v0.33.6/src/zcl/definition/cluster.ts#L2022
//  It is defined as `StelproOutdoorTemp: {ID: 0x4001, type: DataType.INT16}`.
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID 0x4001
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_MIN_VALUE  -3200
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_MAX_VALUE  19900

// Stelpro outdoor temperature:
//  This is Stelpro attribute exposed as a custom attribute.
//  The `StelproSystemMode` attribute definition is available at
//    * https://github.com/Koenkk/zigbee-specification/blob/3c96049cd632d0780b9f79ebc8aecc672d4c4505/src/zcl/definition/clusters.ts#L1905
//    * https://github.com/Koenkk/zigbee-herdsman/blob/v0.33.6/src/zcl/definition/cluster.ts#L2021
//  It is defined as `StelproSystemMode: {ID: 0x401c, type: DataType.ENUM8}`.
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_SYSTEM_MODE
#define ZB_ZCL_ATTR_THERMOSTAT_STELPRO_SYSTEM_MODE_ID 0x401c
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_SYSTEM_MODE

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
typedef struct {
  // Minimum mandatory clusters for a thermostat (see esp_zb_thermostat_cfg_t for list)
  esp_zb_basic_cluster_cfg_t basic_cfg;                                 /*!<  Basic cluster configuration, @ref esp_zb_basic_cluster_cfg_s */
  esp_zb_identify_cluster_cfg_t identify_cfg;                           /*!<  Identify cluster configuration, @ref esp_zb_identify_cluster_cfg_s */
  esp_zb_thermostat_cluster_cfg_t thermostat_cfg;                       /*!<  Thermostat cluster configuration, @ref esp_zb_thermostat_cluster_cfg_t */
  
  // Additional cluster configs
  esp_zb_groups_cluster_cfg_t groups_cfg;                               /*!<  Groups cluster configuration, @ref esp_zb_groups_cluster_cfg_s */
  esp_zb_thermostat_ui_config_cluster_cfg_t thermostat_ui_config_cfg;   /*!<  Thermostat cluster configuration, @ref esp_zb_thermostat_ui_config_cluster_cfg_s */
  //esp_zb_temperature_meas_cluster_cfg_t temp_meas_cfg;                /*!<  Temperature measurement cluster configuration, @ref esp_zb_temperature_meas_cluster_cfg_s */
} zigbee_stelpro_thermostat_cfg_t;

/**
 * @brief Default configuration for Stelpro H420 thermostat
 */

#define ZIGBEE_DEFAULT_STELPRO_THERMOSTAT_CONFIG()                                                    \
{                                                                                                     \
    /* Using same default values as ESP_ZB_DEFAULT_THERMOSTAT_CONFIG() macro */                       \
    .basic_cfg = {                                                                                    \
      .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,                                      \
      .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE,                                    \
    },                                                                                                \
    .identify_cfg = {                                                                                 \
      .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,                               \
    },                                                                                                \
    .thermostat_cfg = {                                                                               \
      .local_temperature = ESP_ZB_ZCL_THERMOSTAT_LOCAL_TEMPERATURE_DEFAULT_VALUE,                     \
      .occupied_cooling_setpoint = ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE,     \
      .occupied_heating_setpoint = ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE,     \
      .control_sequence_of_operation = ESP_ZB_ZCL_THERMOSTAT_CONTROL_SEQ_OF_OPERATION_DEFAULT_VALUE,  \
      .system_mode = ESP_ZB_ZCL_THERMOSTAT_CONTROL_SYSTEM_MODE_DEFAULT_VALUE,                         \
    },                                                                                                \
                                                                                                      \
    .groups_cfg = {                                                                                   \
      .groups_name_support_id = ESP_ZB_ZCL_GROUPS_NAME_SUPPORT_DEFAULT_VALUE,                         \
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
    _local_temperature.onValueChange(callback);
  }
  void onOccupiedCoolSetpointChange(void (*callback)(int16_t)) {
    _occupied_cooling_setpoint.onValueChange(callback);
  }
  void onOccupiedHeatSetpointChange(void (*callback)(int16_t)) {
    _occupied_heating_setpoint.onValueChange(callback);
  }
  void onControlSequenceOfOperationChange(void (*callback)(uint8_t)) {
    _control_sequence_of_operation.onValueChange(callback);
  }
  void onSystemModeChange(void (*callback)(uint8_t)) {
    _system_mode.onValueChange(callback);
  }
  // Thermostat UI cluster
  void onDisplayModeChange(void (*callback)(uint8_t)) {
    _ui_config_display_mode.onValueChange(callback);
  }
  void onKeypadLockoutChange(void (*callback)(uint8_t)) {
    _ui_config_keypad_lockout.onValueChange(callback);
  }
  // Thermostat cluster, additional attributes
  void onRunningStateChange(void (*callback)(uint16_t)) {
    _running_state.onValueChange(callback);
  }
  void onPIHeatingDemandChange(void (*callback)(uint8_t)) {
    _pi_heating_demand.onValueChange(callback);
  }
  void onOutdoorTemperatureChange(void (*callback)(int16_t)) {
    _outdoor_temperature.onValueChange(callback);
  }
  void onOccupancyChange(void (*callback)(zb_uint8_t)) {
    _occupancy.onValueChange(callback);
  } 
  void onStelproOutdoorTemperatureChange(void (*callback)(int16_t)) {
    _stelpro_outdoor_temperature.onValueChange(callback);
  } 

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
  // Manufacturer custom attributes
  bool getStelproOutdoorTemp(int16_t& output) const               { return _stelpro_outdoor_temperature   .get(output); }

  // Zigbee atributes setters
  // Thermostat cluster mandatory attributes
  bool setLocalTemperature(int16_t temperature);
  bool setOccupiedCoolingSetpoint(int16_t setpoint);
  bool setOccupiedHeatingSetpoint(int16_t setpoint);
  bool setControlSequenceOfOperation(uint8_t csop);
  bool setSystemMode(uint8_t mode);
  // Thermostat cluster additional attributes
  bool setRunningState(uint16_t running);
  bool setPIHeatingDemand(uint8_t demand);  // Can only be set when HEATING bit of running_state is set. Must be set to 0 before turning off the HEATING bit. Must be set to non-zero after turning on the HEATING bit.
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
  // Manufacturer custom attributes
  bool setStelproOutdoorTemp(int16_t temperature);

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Power reporting setters
  bool setInstantaneousDemand(int32_t demand_watts);
  bool setCurrentSummationDelivered(uint64_t energy_wh);
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

  // Update energy calculations (call in loop)
  void updateEnergy();

  bool update();
  bool report();
  bool setup();

  void printZigbeeAttributes();

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
  
public:
  typedef struct zb_zcl_stelpro_thermostat_snapshot_s {
    // Thermostat cluster
    int16_t     local_temperature                 ;
    int16_t     occupied_cooling_setpoint         ;
    int16_t     occupied_heating_setpoint         ;
    uint8_t     control_sequence_of_operation     ;
    uint8_t     system_mode                       ;
    // Thermostat cluster additional attributes
    uint16_t    running_state                     ;
    uint8_t     pi_heating_demand                 ;
    int16_t     outdoor_temperature               ;
    zb_uint8_t  occupancy                         ;
    int16_t     min_heat_setpoint_limit           ;
    int16_t     max_heat_setpoint_limit           ;
    int16_t     abs_min_heat_setpoint_limit       ;
    int16_t     abs_max_heat_setpoint_limit       ;
    int16_t     min_cool_setpoint_limit           ;
    int16_t     max_cool_setpoint_limit           ;
    int16_t     abs_min_cool_setpoint_limit       ;
    int16_t     abs_max_cool_setpoint_limit       ;
    // Thermostat UI cluster mandatory attributes
    uint8_t     ui_config_display_mode            ;
    uint8_t     ui_config_keypad_lockout          ;
    // Manufacturer attributes variables
    int16_t     stelpro_outdoor_temperature       ;
  } zb_zcl_stelpro_thermostat_snapshot_t;
  zb_zcl_stelpro_thermostat_snapshot_t _previous = {0};

  bool getSnapshot(zb_zcl_stelpro_thermostat_snapshot_t& snapshot);
  void printSnapshot(const zb_zcl_stelpro_thermostat_snapshot_t& snapshot);

private:
  /*
  zigbee attribute definitions: {
      {name: '_local_temperature', endpoint: 0x19, attr: Local Temperature (0x0000), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Reporting, Read Only (0x0005), size: 2}
      {name: '_occupied_cooling_setpoint', endpoint: 0x19, attr: Occupied Cooling Setpoint (0x0011), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Scene, Read/Write (0x0013), size: 2}
      {name: '_occupied_heating_setpoint', endpoint: 0x19, attr: Occupied Heating Setpoint (0x0012), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Scene, Read/Write (0x0013), size: 2}
      {name: '_control_sequence_of_operation', endpoint: 0x19, attr: Control Sequence Of Operation (0x001b), cluster: Thermostat (0x0201), type: 8-bit Enumeration, access: Read/Write (0x0003), size: 1}
      {name: '_system_mode', endpoint: 0x19, attr: System Mode (0x001c), cluster: Thermostat (0x0201), type: 8-bit Enumeration, access: Scene, Read/Write (0x0013), size: 1}
      {name: '_running_state', endpoint: 0x19, attr: Thermostat Running State (0x0029), cluster: Thermostat (0x0201), type: 16-bit Bitmap, access: Read Only (0x0001), size: 2}
      {name: '_pi_heating_demand', endpoint: 0x19, attr: PI Heating Demand (0x0008), cluster: Thermostat (0x0201), type: Unsigned 8-bit Value, access: Reporting, Read Only (0x0005), size: 1}
      {name: '_outdoor_temperature', endpoint: 0x19, attr: Outdoor Temperature (0x0001), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read Only (0x0001), size: 2}
      {name: '_occupancy', endpoint: 0x19, attr: Occupancy (0x0002), cluster: Thermostat (0x0201), type: 8-bit Bitmap, access: Read Only (0x0001), size: 1}
      {name: '_min_heat_setpoint_limit', endpoint: 0x19, attr: Min Heat Setpoint Limit (0x0015), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read/Write (0x0003), size: 2}
      {name: '_max_heat_setpoint_limit', endpoint: 0x19, attr: Max Heat Setpoint Limit (0x0016), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read/Write (0x0003), size: 2}
      {name: '_abs_min_heat_setpoint_limit', endpoint: 0x19, attr: Abs Min Heat Setpoint Limit (0x0003), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read Only (0x0001), size: 2}
      {name: '_abs_max_heat_setpoint_limit', endpoint: 0x19, attr: Abs Max Heat Setpoint Limit (0x0004), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read Only (0x0001), size: 2}
      {name: '_min_cool_setpoint_limit', endpoint: 0x19, attr: Min Cool Setpoint Limit (0x0017), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read/Write (0x0003), size: 2}
      {name: '_max_cool_setpoint_limit', endpoint: 0x19, attr: Max Cool Setpoint Limit (0x0018), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read/Write (0x0003), size: 2}
      {name: '_abs_min_cool_setpoint_limit', endpoint: 0x19, attr: Abs Min Cool Setpoint Limit (0x0005), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read Only (0x0001), size: 2}
      {name: '_abs_max_cool_setpoint_limit', endpoint: 0x19, attr: Abs Max Cool Setpoint Limit (0x0006), cluster: Thermostat (0x0201), type: Signed 16-bit Value, access: Read Only (0x0001), size: 2}
      {name: '_ui_config_display_mode', endpoint: 0x19, attr: Temperature Display Mode (0x0000), cluster: Thermostat UI Configuration (0x0204), type: 8-bit Enumeration, access: Read/Write (0x0003), size: 1}
      {name: '_ui_config_keypad_lockout', endpoint: 0x19, attr: Keypad Lockout (0x0001), cluster: Thermostat UI Configuration (0x0204), type: 8-bit Enumeration, access: Read/Write (0x0003), size: 1}
  }

  Notes on specific attributes:
    pi_heating_demand:
      * Percentage of heating demand, see ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE & ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE.
      * Zigbee2mqtt assume values are in [0,255] range. Other zigbee projects (including esp-zigbee-sdk library) assume values in [0,100] range.
        * TODO: validate with a real Stelpro thermostat.
      * Can only be set when HEATING bit of running_state is set. Must be set to 0 before turning off the HEATING bit. Must be set to non-zero after turning on the HEATING bit.
    _ui_config_keypad_lockout:
      * Can be set by the device itself but the new value won't be reflected in zigbee2mqtt. zigbee2mqtt might simple not watching the attribute for changes.
        * TODO: validate with a real Stelpro thermostat.
  */

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
  // Manufacturer attributes variables
  ZigbeeAttribute<int16_t>    _stelpro_outdoor_temperature      ;

  // The list of all attributes, for handling attributes in loops  
  ZigbeeAttributeList _zigbee_attribute_list;

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Metering cluster variables
  esp_zb_int24_t _instantaneous_demand;
  esp_zb_uint48_t _current_summation_delivered;
  esp_zb_uint24_t _multiplier;
  esp_zb_uint24_t _divisor;
  uint8_t _unit_of_measure;
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

};
