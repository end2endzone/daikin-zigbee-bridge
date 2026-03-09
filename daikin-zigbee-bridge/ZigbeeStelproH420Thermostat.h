/**
 * @brief This file declares the ZigbeeStelproH420Thermostat class.
 * The class emulates a Zigbee Stelpro H420/HT402 Thermostat endpoint (Hilo thermostat).
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
#include "stelpro_constants.h"

#ifdef USE_ENERGY_CALCULATOR
#include "EnergyCalculator.h"
#endif // #ifdef USE_ENERGY_CALCULATOR


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

/**
 * @brief Zigbee class that implements a Stelpro H420 thermostat.
 */
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
  void onStelproSystemModeChange(void (*callback)(uint8_t)) {
    _stelpro_system_mode.onValueChange(callback);
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
  bool getStelproOutdoorTemp(int16_t& output) const             { return _stelpro_outdoor_temperature   .get(output); }
  bool getStelproSystemMode(uint8_t& output) const              { return _stelpro_system_mode           .get(output); }
  bool getStelproPower(uint16_t& output) const                  { return _stelpro_power                 .get(output); }
  bool getStelproEnergy(uint32_t& output) const                 { return _stelpro_energy                .get(output); }

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
  bool setStelproSystemMode(uint8_t mode);
  bool setStelproPower(uint16_t output);
  bool setStelproEnergy(uint32_t output);

  // Update energy calculations (call in loop)
  void updateEnergy();

  bool update();
  bool report();
  bool setup();

  void printZigbeeAttributes();

private:
  void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;
  bool updateSystemModes(ZigbeeAttribute<uint8_t> * source, ZigbeeAttribute<uint8_t> * target);

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
  #pragma pack(push, 1)
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
    uint8_t     stelpro_system_mode               ;
    uint16_t    stelpro_power                     ;
    uint32_t    stelpro_energy                    ;

  } zb_zcl_stelpro_thermostat_snapshot_t;
  #pragma pack(pop)

  bool getSnapshot(zb_zcl_stelpro_thermostat_snapshot_t& snapshot);
  void printSnapshot(const zb_zcl_stelpro_thermostat_snapshot_t& snapshot);

private:
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
  ZigbeeAttribute<uint8_t>    _stelpro_system_mode              ;
  ZigbeeAttribute<uint16_t>   _stelpro_power                    ;
  ZigbeeAttribute<uint32_t>   _stelpro_energy                   ;

  // The list of all attributes, for handling attributes in loops  
  ZigbeeAttributeList _zigbee_attribute_list;

};
