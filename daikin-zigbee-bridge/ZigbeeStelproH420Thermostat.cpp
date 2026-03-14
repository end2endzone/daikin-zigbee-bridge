#include "zcl/esp_zigbee_zcl_common.h"
#include "ZigbeeStelproH420Thermostat.h"

#include "esp_zigbee_cluster.h"
#include "zcl/esp_zigbee_zcl_thermostat.h"
#include "zb_constants_helper.h"
#include "logging.h"
#include "zb_debug.h"
#include "zb_helper.h"

void logUnexpectedDataTypeReceived(uint16_t cluster_id, esp_zb_zcl_attr_type_t actual_type_id, uint16_t expected_type_id, uint16_t attr_id) {
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;
  esp_zb_zcl_attr_type_t actual_type = (esp_zb_zcl_attr_type_t)actual_type_id;
  esp_zb_zcl_attr_type_t expected_type = (esp_zb_zcl_attr_type_t)expected_type_id;

  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* actual_name = zb_constants_zcl_attr_type_to_string(actual_type);
  const char* expected_name = zb_constants_zcl_attr_type_to_string(expected_type);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  log_w("Unexpected data type received: cluster=%s, attr=%s, expected=%s, actual=%s", cluster_name, attr_name, expected_name, actual_name);
}

void logUnhandledMessageError(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, uint16_t type_id) {
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;

  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  log_w("Received unknown message: endpoint=%d, cluster=%s (0x%04x), attr=%s (0x%04x), type=%s (0x%04x)",
    endpoint, cluster_name, cluster_id, attr_name, attr_id, zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)type_id), type_id );
}

ZigbeeStelproH420Thermostat::ZigbeeStelproH420Thermostat(uint8_t endpoint) : ZigbeeEP(endpoint) {
  // Disable timers
  _stelpro_peak_demand_icon_timer.setTimeOutTime(0);
  _stelpro_peak_demand_icon_timer.reset();
  _energy_computation_timer.setTimeOutTime(STELPRO_ENERGY_UPDATE_INTERVAL * 1000);
  _energy_computation_timer.reset();

  _device_id = ESP_ZB_HA_THERMOSTAT_DEVICE_ID;
  
  // Init all attributes
  _local_temperature                          .init("_local_temperature"                     , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID);                           
  _occupied_cooling_setpoint                  .init("_occupied_cooling_setpoint"             , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID);                   
  _occupied_heating_setpoint                  .init("_occupied_heating_setpoint"             , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID);                   
  _control_sequence_of_operation              .init("_control_sequence_of_operation"         , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID);               
  _system_mode                                .init("_system_mode"                           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID);                                 
  _running_state                              .init("_running_state"                         , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID);                    
  _pi_heating_demand                          .init("_pi_heating_demand"                     , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID);                           
  _outdoor_temperature                        .init("_outdoor_temperature"                   , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID);                         
  _occupancy                                  .init("_occupancy"                             , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID);                                   
  _min_heat_setpoint_limit                    .init("_min_heat_setpoint_limit"               , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID);                     
  _max_heat_setpoint_limit                    .init("_max_heat_setpoint_limit"               , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID);                     
  _abs_min_heat_setpoint_limit                .init("_abs_min_heat_setpoint_limit"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID);                 
  _abs_max_heat_setpoint_limit                .init("_abs_max_heat_setpoint_limit"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID);                 
  _min_cool_setpoint_limit                    .init("_min_cool_setpoint_limit"               , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID);                     
  _max_cool_setpoint_limit                    .init("_max_cool_setpoint_limit"               , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID);                     
  _abs_min_cool_setpoint_limit                .init("_abs_min_cool_setpoint_limit"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID);                 
  _abs_max_cool_setpoint_limit                .init("_abs_max_cool_setpoint_limit"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID);                 
  _ui_config_display_mode                     .init("_ui_config_display_mode"                , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID);
  _ui_config_keypad_lockout                   .init("_ui_config_keypad_lockout"              , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID);          
  _stelpro_outdoor_temperature                .init("_stelpro_outdoor_temperature"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_STELPRO_ATTR_OUTDOOR_TEMP_ID);
  _stelpro_system_mode                        .init("_stelpro_system_mode"                   , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_STELPRO_ATTR_SYSTEM_MODE_ID);                                 
  _stelpro_power                              .init("_stelpro_power"                         , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_STELPRO_ATTR_POWER_ID);                                 
  _stelpro_energy                             .init("_stelpro_energy"                        , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_STELPRO_ATTR_ENERGY_ID);                                 
  _stelpro_peak_demand_icon                   .init("_stelpro_peak_demand_icon"              , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_STELPRO_ATTR_PEAK_DEMAND_ICON_ID);                                 

  // Fill the attribute list to allowing processing all fields
  _zigbee_attribute_list.push_back(&_local_temperature             );
  _zigbee_attribute_list.push_back(&_occupied_cooling_setpoint     );
  _zigbee_attribute_list.push_back(&_occupied_heating_setpoint     );
  _zigbee_attribute_list.push_back(&_control_sequence_of_operation );
  _zigbee_attribute_list.push_back(&_system_mode                   );
  _zigbee_attribute_list.push_back(&_running_state                 );
  _zigbee_attribute_list.push_back(&_pi_heating_demand             );
  _zigbee_attribute_list.push_back(&_outdoor_temperature           );
  _zigbee_attribute_list.push_back(&_occupancy                     );
  _zigbee_attribute_list.push_back(&_min_heat_setpoint_limit       );
  _zigbee_attribute_list.push_back(&_max_heat_setpoint_limit       );
  _zigbee_attribute_list.push_back(&_abs_min_heat_setpoint_limit   );
  _zigbee_attribute_list.push_back(&_abs_max_heat_setpoint_limit   );
  _zigbee_attribute_list.push_back(&_min_cool_setpoint_limit       );
  _zigbee_attribute_list.push_back(&_max_cool_setpoint_limit       );
  _zigbee_attribute_list.push_back(&_abs_min_cool_setpoint_limit   );
  _zigbee_attribute_list.push_back(&_abs_max_cool_setpoint_limit   );
  _zigbee_attribute_list.push_back(&_ui_config_display_mode        );
  _zigbee_attribute_list.push_back(&_ui_config_keypad_lockout      );
  _zigbee_attribute_list.push_back(&_stelpro_outdoor_temperature   );
  _zigbee_attribute_list.push_back(&_stelpro_system_mode           );
  _zigbee_attribute_list.push_back(&_stelpro_power                 );
  _zigbee_attribute_list.push_back(&_stelpro_energy                );
  _zigbee_attribute_list.push_back(&_stelpro_peak_demand_icon      );

  // Assert all attributes are initialized
  {
    for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
      IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
      if (!attr_p->isInitialized()) {
        log_w("Attribute [%d] is not initialized: %s", i, attr_p->toString().c_str());
      }
    }
  }

  // Set zigbee default attribute initialization values.
  // The following zigbee attributes are custom and not created by esp_zb_thermostat_cluster_create()
  // We need to initialize their default value before they are created in the zigbee stack.
  _running_state                              .setDefaultValue(THERMOSTAT_RUNNING_STATE_IDLE);
  _pi_heating_demand                          .setDefaultValue(0);
  _outdoor_temperature                        .setDefaultValue(0);
  _occupancy                                  .setDefaultValue(ESP_ZB_ZCL_THERMOSTAT_OCCUPANCY_DEFAULT_VALUE);
  _ui_config_display_mode                     .setDefaultValue(ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE);
  _ui_config_keypad_lockout                   .setDefaultValue(ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_DEFAULT_VALUE);
  _stelpro_outdoor_temperature                .setDefaultValue(0);
  _stelpro_system_mode                        .setDefaultValue(ESP_ZB_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT);
  _stelpro_power                              .setDefaultValue(0);
  _stelpro_energy                             .setDefaultValue(0);
  _stelpro_peak_demand_icon                   .setDefaultValue(0);

  // Build cluster lists
  {
    zigbee_stelpro_thermostat_cfg_t stelpro_cfg = ZIGBEE_DEFAULT_STELPRO_THERMOSTAT_CONFIG();

    // Change default values from macro ESP_ZB_DEFAULT_THERMOSTAT_CONFIG()
    {
      // Change from basic power source to constant power    
      stelpro_cfg.basic_cfg.power_source = ESP_ZB_AF_NODE_POWER_SOURCE_CONSTANT_POWER;
      
      // For `occupied_cooling_setpoint`, do not use default value `ESP_ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE` of 26.0°C.
      // If you do, this will set an upper limit for `occupied_heating_setpoint` at 25.0°C.
      // Controllers that try to set a value above 25.0°C will get a "Invalid Value" error such as:
      // ```
      // z2m: Publish 'set' 'occupied_heating_setpoint' to '0x414cbaffde563423' failed:
      // 'Error: ZCL command 0x414cbaffde563423/25 hvacThermostat.write(
      // {"occupiedHeatingSetpoint":2900}, 
      // {"timeout":10000,
      //   "disableResponse":false,
      //   "disableRecovery":false,
      //   "disableDefaultResponse":true,
      //   "direction":0,
      //   "reservedBits":0,
      //   "writeUndiv":false})
      //   failed (Status 'INVALID_VALUE')'
      // ```
      // I think the zigbee stack can not allow a situation where `occupied_heating_setpoint > occupied_cooling_setpoint`.
      stelpro_cfg.thermostat_cfg.occupied_cooling_setpoint = STELPRO_OCCUPIED_COOLING_SETPOINT;

      // Change from `ESP_ZB_ZCL_THERMOSTAT_CONTROL_SYSTEM_MODE_DEFAULT_VALUE` which is `ESP_ZB_ZCL_THERMOSTAT_SYSTEM_MODE_AUTO`
      // to `ESP_ZB_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT`.
      stelpro_cfg.thermostat_cfg.system_mode = ESP_ZB_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT;
    }
    
    // Set cluster list in base class
    _cluster_list = zigbee_stelpro_thermostat_clusters_create(&stelpro_cfg);
  }

  // DEBUG:
  // Print the list of all clusters and attributes...
  log_i("Clusters summary:");
  zb_print_markdown_cluster_list_summary(_cluster_list);
  log_i("Attributes summary:");
  zb_print_markdown_attributes_summary(_cluster_list);

  // Set zigbee default attribute initialization values.
  // The following zigbee attributes are attributes automatically created by esp_zb_thermostat_cluster_create() and other similar functions.
  // We need to initialize their default value before they are created in the zigbee stack.
  if (!zb_set_attribute_value_in_cluster_list<uint16_t>   (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID            , THERMOSTAT_RUNNING_STATE_IDLE))                                             log_w("Failed to set attribute 'RUNNING_STATE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID                   , 0))                                                                         log_w("Failed to set attribute 'PI_HEATING_DEMAND' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<int16_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID                 , 0 ))                                                                        log_w("Failed to set attribute 'OUTDOOR_TEMPERATURE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<zb_uint8_t> (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID                           , ESP_ZB_ZCL_THERMOSTAT_OCCUPANCY_DEFAULT_VALUE ))                            log_w("Failed to set attribute 'OCCUPANCY' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG  , ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID  , ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE ))   log_w("Failed to set attribute 'DISPLAY_MODE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG  , ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID            , ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_DEFAULT_VALUE ))             log_w("Failed to set attribute 'KEYPAD_LOCKOUT' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<int16_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_STELPRO_ATTR_OUTDOOR_TEMP_ID                                   , 0 ))                                                                        log_w("Failed to set attribute 'STELPRO_OUTDOOR_TEMPERATURE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_STELPRO_ATTR_SYSTEM_MODE_ID                                    , ESP_ZB_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT ))                                   log_w("Failed to set attribute 'STELPRO_SYSTEM_MODE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint16_t>   (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_STELPRO_ATTR_POWER_ID                                          , 0 ))                                                                        log_w("Failed to set attribute 'STELPRO_POWER' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint32_t>   (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_STELPRO_ATTR_ENERGY_ID                                         , 0 ))                                                                        log_w("Failed to set attribute 'STELPRO_ENERGY' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint16_t>   (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_STELPRO_ATTR_PEAK_DEMAND_ICON_ID                              , 0 ))                                                                         log_w("Failed to set attribute 'STELPRO_PEAK_DEMAND_ICON' value in cluster list!");

  // Set endpoint configuration
  _ep_config = {
    .endpoint = _endpoint,
    .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
    .app_device_id = ESP_ZB_HA_THERMOSTAT_DEVICE_ID,
    .app_device_version = 0
  };

}

void ZigbeeStelproH420Thermostat::zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {
  // Handle incoming Zigbee attribute changes.
  // When this function is called, the internal memory of the attribute is already updated.
  // There is no need to call setters.
  
  // Find the attribute that was updated
  IZigbeeAttribute* attr = findAttribute(_zigbee_attribute_list, message->info.dst_endpoint, message->info.cluster, message->attribute.id);
  if (!attr) {
    logUnhandledMessageError(message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.type);
    return;
  }

  // Assert the attribute's type
  esp_zb_zcl_attr_type_t update_type_id = (esp_zb_zcl_attr_type_t)message->attribute.data.type;
  esp_zb_zcl_attr_type_t attr_type_id = attr->getTypeId();
  if (update_type_id != attr_type_id) {
    logUnexpectedDataTypeReceived(message->info.cluster, attr_type_id, update_type_id, message->attribute.id);
    return;
  }

  // Special case for system modes attributes
  if (attr == &_stelpro_system_mode) {
    // _stelpro_system_mode was updated from the coordinator
    // Update _system_mode from _stelpro_system_mode
    updateSystemModes(&_stelpro_system_mode, &_system_mode);
  } else if (attr == &_system_mode) {
    // _system_mode was updated from the coordinator
    // Update _stelpro_system_mode from _system_mode
    updateSystemModes(&_system_mode, &_stelpro_system_mode);
  }

  // Special case for StelproPeakDemandIcon
  if (attr == &_stelpro_peak_demand_icon) {
    // Start matching timer and start counting from this point
    _stelpro_peak_demand_icon_timer.setTimeOutTime(STELPRO_PEAK_DEMAND_ICON_UPDATE_INTERVAL * 1000);
    _stelpro_peak_demand_icon_timer.reset();
  }

  // Check if value has changed. Ignore the attribute set command if not changed.
  if (!attr->hasChanged()) {
    return;
  }

  // Notify observers of the change
  attr->notifyChange();
}

bool ZigbeeStelproH420Thermostat::updateSystemModes(ZigbeeAttribute<uint8_t> * source, ZigbeeAttribute<uint8_t> * target) {
  uint8_t mode = 0;

  // Get source value from the attribute's internal memory pointer.
  // Use getUnsafe() to not aquire/release a lock on the data to allow calling this function within a callback.
  bool success = source->getUnsafe(mode);
  if (!success) {
    log_e("*** Failure in updateSystemModes(). Failed calling getUnsafe() from '%s'.", source->getName());
    return false;
  }

  success = target->setUnsafe(mode);
  if (!success) {
    log_e("*** Failure in updateSystemModes(). Failed calling setUnsafe() from '%s'.", source->getName());
    return false;
  }

  // Note: '_system_mode' and '_stelpro_system_mode' can not be reported.

  return true;
}

// Zigbee attribute setters
// Thermostat cluster mandatory attributes
bool ZigbeeStelproH420Thermostat::setLocalTemperature(int16_t temperature) {
  //// Round value to the nearest multiple of STELPRO_TEMP_MEASUREMENT_TOLERANCE
  //temperature = (((temperature + (STELPRO_TEMP_MEASUREMENT_TOLERANCE/2)) / STELPRO_TEMP_MEASUREMENT_TOLERANCE) * STELPRO_TEMP_MEASUREMENT_TOLERANCE);

  bool success = _local_temperature.set(temperature);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedCoolingSetpoint(int16_t setpoint) {
  bool success = _occupied_cooling_setpoint.set(setpoint);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedHeatingSetpoint(int16_t setpoint) {
  bool success = _occupied_heating_setpoint.set(setpoint);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setControlSequenceOfOperation(uint8_t csop) {
  bool success = _control_sequence_of_operation.set(csop);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setSystemMode(uint8_t mode) {
  bool success = _system_mode.set(mode);
  if (!success)
    return false;
  return success;
}

// Thermostat cluster, additional attributes
bool ZigbeeStelproH420Thermostat::setRunningState(uint16_t state) {
  bool success = _running_state.set(state);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setPIHeatingDemand(uint8_t demand) {
  bool success = _pi_heating_demand.set(demand);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setOutdoorTemperature(int16_t temperature) {
  bool success = _outdoor_temperature.set(temperature);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupancy(zb_uint8_t occupancy) {
  bool success = _occupancy.set(occupancy);
  if (!success)
    return false;
  return success;
}

// Thermostat UI cluster
bool ZigbeeStelproH420Thermostat::setTemperatureDisplayMode(uint8_t mode) {
  bool success = _ui_config_display_mode.set(mode);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setKeypadLockout(uint8_t lockout) {
  bool success = _ui_config_keypad_lockout.set(lockout);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setStelproOutdoorTemp(int16_t temperature) {
  bool success = _stelpro_outdoor_temperature.set(temperature);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setStelproSystemMode(uint8_t mode) {
  // Do not update _stelpro_system_mode attribute.
  // Update _system_mode attribute and then synchronize _stelpro_system_mode to _system_mode.
  bool success = _system_mode.set(mode);
  if (!success)
    return false;
  updateSystemModes(&_system_mode, &_stelpro_system_mode);
  return success;
}

bool ZigbeeStelproH420Thermostat::setStelproPower(uint16_t value) {
  bool success = _stelpro_power.set(value);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setStelproEnergy(uint32_t value) {
  bool success = _stelpro_energy.set(value);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::setStelproPeakDemandIcon(uint16_t value) {
  bool success = _stelpro_peak_demand_icon.set(value);
  if (!success)
    return false;
  return success;
}

bool ZigbeeStelproH420Thermostat::updateHeatingLogic() {
  // Capture actual intput value
  zb_zcl_stelpro_thermostat_snapshot_t input = {0};
  if (!getSnapshot(input))
    return false;
  zb_zcl_stelpro_thermostat_snapshot_t output = input;

  int16_t temp_diff = input.occupied_heating_setpoint - input.local_temperature; // positive when requiring heating
  if (abs(temp_diff) < STELPRO_TEMPERATURE_DIFFERENCE_THRESHOLD) {
    // Nothing to do.
    // Set everything to OFF, temperature do not need to change.
    output.pi_heating_demand = 0;
    output.running_state = THERMOSTAT_RUNNING_STATE_IDLE;
    output.stelpro_power = 0;
  } else {
    // Temperature difference is above the threshold. Temperature must change and system must be updated.

    // Compute new system mode based on current situation setpoint vs local_temp.
    if (input.occupied_heating_setpoint > input.local_temperature) {
      // Thermostat must HEAT
      output.running_state |= ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT;
    } else {
      output.running_state = THERMOSTAT_RUNNING_STATE_IDLE;
      output.stelpro_power = 0;
    }

    // Compute new pi_heating_demand
    if ((output.running_state | ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT) > 0) {
      // map [0°C,3°C] to [0%,100%]
      int16_t tmp = (int16_t)map(
        abs(temp_diff),
        0000,
        0300,
        (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE,
        (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE);
      if (tmp > 100) tmp = 100;
      if (tmp < 0) tmp = 0;
      output.pi_heating_demand = (uint8_t)tmp;

      // Compute new power
      // map [0%,100%] to [0W,4000W]
      output.stelpro_power = (uint16_t)map(
        output.pi_heating_demand,
        (uint16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE,
        (uint16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE,
        0,
        STELPRO_MAX_POWER_WATTS);
    }
  }

  // Note:
  // Zigbee attribute `pi_heating_demand` can not be set to a value (even 0) if :
  // * attribute `system_mode` is IDLE or
  // * HEATING bit in `running_state` is not set.
  // So we must set this flag sometimes before (and sometimes after) `system_mode` and `running_state` to make sure it is accepted.
  // Using the following dirty bit to know if an update is required 
  bool pi_heating_demand_is_dirty = (output.pi_heating_demand != input.pi_heating_demand);

  // Updating `pi_heating_demand` and `stelpro_power`
  uint16_t tmp_running_state = 0;
  if (pi_heating_demand_is_dirty && this->getRunningState(tmp_running_state) && (tmp_running_state & ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT)) {
    // If heating bit is already set, it is safe to update pi_heating_demand now.
    // This cover use case when `running_state` transitions from `HEATING` --> `IDLE`.
    // We must update `pi_heating_demand` and `stelpro_power` before HEATING flag in `runnig_state` is cleared.
    if (!this->setPIHeatingDemand(output.pi_heating_demand)) {
      log_e("*** Failed updating attribute 'pi_heating_demand' to '%u'.", output.pi_heating_demand);
      return false;
    }
    pi_heating_demand_is_dirty = false;

    // Update current power usage
    if (!this->setStelproPower(output.stelpro_power)) {
      log_e("*** Failed updating attribute 'stelpro_power' to '%u'.", output.stelpro_power);
      return false;
    }

    // Log the change
    log_i("Heating logic update:  Demand: %d%%, Power: %dW",
                  output.pi_heating_demand,
                  output.stelpro_power);
  }

  // Update thermostat state, if changed
  bool running_state_dirty = (output.running_state != input.running_state);
  if (running_state_dirty) {
    if (!this->setRunningState(output.running_state)) {
      log_e("*** Failed updating attribute 'running_state' to '%u'.", output.running_state);
      return false;
    }

    // Log the change
    log_i("Heating logic update:  State: %s", zb_constants_zcl_thermostat_running_state_attr_to_string(output.running_state).c_str());
  }
  
  // If pi_heating_demand is still dirty, we must update it now
  if (pi_heating_demand_is_dirty) {
    // If pi_heating_demand is still dirty, we update it.
    // This cover use case when `running_state` has just transitioned from `IDLE` --> `HEATING`.
    // We must update `pi_heating_demand` after HEATING flag in `runnig_state` is set.
    if (!this->setPIHeatingDemand(output.pi_heating_demand)) {
      log_e("*** Failed updating attribute 'pi_heating_demand' to '%u'.", output.pi_heating_demand);
      return false;
    }
    pi_heating_demand_is_dirty = false;

    // Update current power usage
    if (!this->setStelproPower(output.stelpro_power)) {
      log_e("*** Failed updating attribute 'stelpro_power' to '%u'.", output.stelpro_power);
      return false;
    }

    // Log the change
    log_i("Heating logic update:  Demand: %d%%, Power: %dW",
                  output.pi_heating_demand,
                  output.stelpro_power);

  }

  return true;
}

bool ZigbeeStelproH420Thermostat::updateEnergy() {
  // Make sure we do not call this function too often...
  if (_energy_computation_timer.getTimeOutTime() != 0 && !_energy_computation_timer.hasTimedOut()) {
    return true; // too soon
  }
  // reset timer for next iteration timestamps
  _energy_computation_timer.reset();

  // Capture actual intput value
  zb_zcl_stelpro_thermostat_snapshot_t input = {0};
  if (!getSnapshot(input))
    return false;
  zb_zcl_stelpro_thermostat_snapshot_t output = input;

  // Does energy monitoring needs update ?
  if (input.pi_heating_demand == 0 || input.stelpro_power == 0) {
    return true; // nothing to do
  }

  // Compute energy use since last update. Use the previous power value to compute output energy.
  static const double MILLISECONDS_TO_SECONDS = 0.001;
  static const double SECONDS_TO_HOURS = 1/3600.0;
  double elapsed_hours_since_last_update = _energy_computation_timer.getTimeOutTime()  * MILLISECONDS_TO_SECONDS * SECONDS_TO_HOURS;
  uint32_t energy_use_deltaT = (uint32_t)(input.stelpro_power * elapsed_hours_since_last_update);
  output.stelpro_energy = input.stelpro_energy + energy_use_deltaT;

  log_i("Energy consumption update:  Increasing total energy by %uW, going from %uW to %uW",
    energy_use_deltaT,
    input.stelpro_energy,
    output.stelpro_energy);

  // Update Energy
  bool stelpro_energy_is_dirty = (output.stelpro_energy != input.stelpro_energy);
  if (input.pi_heating_demand > 0 && stelpro_energy_is_dirty) {
    if (!this->setStelproEnergy(output.stelpro_energy)) {
      log_e("*** Failed updating attribute 'stelpro_energy' to '%u'.", output.stelpro_energy);
      return false;
    }

    // Energy update successful
    stelpro_energy_is_dirty = false;

    // Reset timer to the next energy update
    _energy_computation_timer.reset();
  }

  return true;
}

bool ZigbeeStelproH420Thermostat::update() {
  if (!updatePreviousAttributeValues())
    return false;
  if (!updateStelproPeakDemandIcon())
    return false;
  if (!updateHeatingLogic())
    return false;
  if (!updateEnergy())
    return false;
  return true;
}

bool ZigbeeStelproH420Thermostat::updatePreviousAttributeValues() {
  // Remember previous values
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
    if (!attr_p->update()) {
      log_w("Attribute [%d] is failed to update: %s", i, attr_p->toString().c_str());
    }
  }
  return true;
}

bool ZigbeeStelproH420Thermostat::updateStelproPeakDemandIcon() {
  // Check if an update for StelproPeakDemandIcon is required
  if (_stelpro_peak_demand_icon_timer.getTimeOutTime() != 0 && _stelpro_peak_demand_icon_timer.hasTimedOut()) { // if active and has timed out
    uint16_t remaining = 0;
    getStelproPeakDemandIcon(remaining); // don't care if the getter has worked or not

    // 1 round of timer has elapsed
    if (remaining > STELPRO_PEAK_DEMAND_ICON_UPDATE_INTERVAL) {
      remaining -= STELPRO_PEAK_DEMAND_ICON_UPDATE_INTERVAL;
    } else {
      remaining = 0;
    }

    //// DEBUG:
    //if (remaining == 3570) {
    //  remaining = 20;
    //}

    setStelproPeakDemandIcon(remaining);

    // If the event is done
    if (remaining == 0) {
      // Disable the timer
      _stelpro_peak_demand_icon_timer.setTimeOutTime(0);
    }

    // Start counting from here again
    _stelpro_peak_demand_icon_timer.reset();
  }

  return true;
}

bool ZigbeeStelproH420Thermostat::report() {
  bool success = true;
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];

    esp_zb_zcl_attr_access_t access_id = attr_p->getAccessId();
    bool isReportable = ((access_id & ESP_ZB_ZCL_ATTR_ACCESS_REPORTING) > 0);
    if (isReportable) {
      bool hasReported = attr_p->report();

      // Assert attribute has reported properly
      if (!hasReported) {
        // No need to print an error, attr_p->report() has already printed a warning.
      }

      // show a warning once if any attribute has failed reporting.
      success = success && hasReported;
    }
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setup() {
  bool success = true;

  // Setup all attributes
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
    bool isReady = attr_p->setup();

    // Assert attribute is setup() properly
    if (!isReady) {
      log_e("Attribute [%d] has failed to setup(): %s", i, attr_p->toString().c_str());
    }
    success = success && isReady;
  }

  if (!success) {
    log_w("************************************************************************************");
    log_w("*                                                                                  *");
    log_w("*                                                                                  *");
    log_w("*                   Some zigbee attributes has failed to setup!                    *");
    log_w("*                                                                                  *");
    log_w("*                                                                                  *");
    log_w("************************************************************************************");
  }

  log_i("attributes: {");
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
    log_i("    %s", attr_p->toString().c_str());
  }
  log_i("}");
  return success;
}

void ZigbeeStelproH420Thermostat::printZigbeeAttributes() {
  log_i("-----> _local_temperature            .get()=%d", _local_temperature            .get());
  log_i("-----> _occupied_cooling_setpoint    .get()=%d", _occupied_cooling_setpoint    .get());
  log_i("-----> _occupied_heating_setpoint    .get()=%d", _occupied_heating_setpoint    .get());
  log_i("-----> _control_sequence_of_operation.get()=%d", _control_sequence_of_operation.get());
  log_i("-----> _system_mode                  .get()=%d", _system_mode                  .get());
  log_i("-----> _running_state                .get()=%d", _running_state                .get());
  log_i("-----> _pi_heating_demand            .get()=%d", _pi_heating_demand            .get());
  log_i("-----> _outdoor_temperature          .get()=%d", _outdoor_temperature          .get());
  log_i("-----> _occupancy                    .get()=%d", _occupancy                    .get());
  log_i("-----> _min_heat_setpoint_limit      .get()=%d", _min_heat_setpoint_limit      .get());
  log_i("-----> _max_heat_setpoint_limit      .get()=%d", _max_heat_setpoint_limit      .get());
  log_i("-----> _abs_min_heat_setpoint_limit  .get()=%d", _abs_min_heat_setpoint_limit  .get());
  log_i("-----> _abs_max_heat_setpoint_limit  .get()=%d", _abs_max_heat_setpoint_limit  .get());
  log_i("-----> _min_cool_setpoint_limit      .get()=%d", _min_cool_setpoint_limit      .get());
  log_i("-----> _max_cool_setpoint_limit      .get()=%d", _max_cool_setpoint_limit      .get());
  log_i("-----> _abs_min_cool_setpoint_limit  .get()=%d", _abs_min_cool_setpoint_limit  .get());
  log_i("-----> _abs_max_cool_setpoint_limit  .get()=%d", _abs_max_cool_setpoint_limit  .get());
  log_i("-----> _ui_config_display_mode       .get()=%d", _ui_config_display_mode       .get());
  log_i("-----> _ui_config_keypad_lockout     .get()=%d", _ui_config_keypad_lockout     .get());
  log_i("-----> _stelpro_outdoor_temperature  .get()=%d", _stelpro_outdoor_temperature  .get());
  log_i("-----> _stelpro_system_mode          .get()=%d", _stelpro_system_mode          .get());
  log_i("-----> _stelpro_power                .get()=%d", _stelpro_power                .get());
  log_i("-----> _stelpro_energy               .get()=%d", _stelpro_energy               .get());
  log_i("-----> _stelpro_peak_demand_icon     .get()=%d", _stelpro_peak_demand_icon     .get());
}

bool ZigbeeStelproH420Thermostat::getSnapshot(zb_zcl_stelpro_thermostat_snapshot_t& snapshot) {
  bool success = true;
  size_t capture_size = 0;

  // Capture all attribute values in a single operation, using a single lock.
  // Using this class' getters isn't suitable because each one acquires and releases
  // the lock independently, causing the attributes to be captured  one by one rather
  // than atomically.
  // Repeatedly locking and unlocking during the capture window creates a risk that
  // an attribute may change between getter calls.
  // To avoid this, acquire the lock once, read every attribute, then release it.

  esp_zb_lock_acquire(portMAX_DELAY);

  // Define capture logic.
  #define CAPTURE_ATTR(attribute, output)                                                         \
    capture_size += attribute.getSize();                                                          \
    if (!attribute.getUnsafe( output )) {                                                         \
      log_w("Failed to get attribute in snapshot: %s", attribute.toString().c_str());             \
      success = false;                                                                            \
    }

  // Thermostat cluster mandatory attributes
  CAPTURE_ATTR(_local_temperature                , snapshot.local_temperature                 );
  CAPTURE_ATTR(_occupied_cooling_setpoint        , snapshot.occupied_cooling_setpoint         );
  CAPTURE_ATTR(_occupied_heating_setpoint        , snapshot.occupied_heating_setpoint         );
  CAPTURE_ATTR(_control_sequence_of_operation    , snapshot.control_sequence_of_operation     );
  CAPTURE_ATTR(_system_mode                      , snapshot.system_mode                       );
  // Thermostat cluster additional attributes
  CAPTURE_ATTR(_running_state                    , snapshot.running_state                     );
  CAPTURE_ATTR(_pi_heating_demand                , snapshot.pi_heating_demand                 );
  CAPTURE_ATTR(_outdoor_temperature              , snapshot.outdoor_temperature               );
  CAPTURE_ATTR(_occupancy                        , snapshot.occupancy                         );
  CAPTURE_ATTR(_min_heat_setpoint_limit          , snapshot.min_heat_setpoint_limit           );
  CAPTURE_ATTR(_max_heat_setpoint_limit          , snapshot.max_heat_setpoint_limit           );
  CAPTURE_ATTR(_abs_min_heat_setpoint_limit      , snapshot.abs_min_heat_setpoint_limit       );
  CAPTURE_ATTR(_abs_max_heat_setpoint_limit      , snapshot.abs_max_heat_setpoint_limit       );
  CAPTURE_ATTR(_min_cool_setpoint_limit          , snapshot.min_cool_setpoint_limit           );
  CAPTURE_ATTR(_max_cool_setpoint_limit          , snapshot.max_cool_setpoint_limit           );
  CAPTURE_ATTR(_abs_min_cool_setpoint_limit      , snapshot.abs_min_cool_setpoint_limit       );
  CAPTURE_ATTR(_abs_max_cool_setpoint_limit      , snapshot.abs_max_cool_setpoint_limit       );
  // Thermostat UI cluster mandatory attributes
  CAPTURE_ATTR(_ui_config_display_mode           , snapshot.ui_config_display_mode            );
  CAPTURE_ATTR(_ui_config_keypad_lockout         , snapshot.ui_config_keypad_lockout          );
  // Manufacturer attributes variables
  CAPTURE_ATTR(_stelpro_outdoor_temperature      , snapshot.stelpro_outdoor_temperature       );
  CAPTURE_ATTR(_stelpro_system_mode              , snapshot.stelpro_system_mode               );
  CAPTURE_ATTR(_stelpro_power                    , snapshot.stelpro_power                     );
  CAPTURE_ATTR(_stelpro_energy                   , snapshot.stelpro_energy                    );
  CAPTURE_ATTR(_stelpro_peak_demand_icon         , snapshot.stelpro_peak_demand_icon          );

  #undef CAPTURE_ATTR

  esp_zb_lock_release();

  // Assert that all snapshot variables were captured
  // (assert total size of all attributes matches the size of the snapshot struct)
  size_t snapshot_size = sizeof(snapshot);
  if (success && capture_size != snapshot_size) {
    log_e("Failed to get all attributes in snapshot: snapshot_size=%d, capture_size=%d", snapshot_size, capture_size);
    return false;
  }

  return success;
}

void ZigbeeStelproH420Thermostat::printSnapshot(const zb_zcl_stelpro_thermostat_snapshot_t& snapshot) {
  log_i("    _local_temperature            = %d,  %.1f°C",     snapshot.local_temperature            ,     snapshot.local_temperature        /100.0    );
  log_i("    _occupied_cooling_setpoint    = %d,  %.1f°C",     snapshot.occupied_cooling_setpoint    ,     snapshot.occupied_cooling_setpoint/100.0    );
  log_i("    _occupied_heating_setpoint    = %d,  %.1f°C",     snapshot.occupied_heating_setpoint    ,     snapshot.occupied_heating_setpoint/100.0    );
  log_i("    _control_sequence_of_operation= %d",              snapshot.control_sequence_of_operation);
  log_i("    _system_mode                  = %d,  %s",         snapshot.system_mode                  ,     zb_constants_zcl_thermostat_system_mode_attr_to_string((esp_zb_zcl_thermostat_system_mode_t)snapshot.system_mode));
  log_i("    _running_state                = %d,  %s",         snapshot.running_state                ,     zb_constants_zcl_thermostat_running_state_attr_to_string(snapshot.running_state).c_str());
  log_i("    _pi_heating_demand            = %d%%",            snapshot.pi_heating_demand            );
  log_i("    _outdoor_temperature          = %d,  %.1f°C",     snapshot.outdoor_temperature          ,     snapshot.outdoor_temperature      /100.0    );
  log_i("    _occupancy                    = %d",              snapshot.occupancy                    );
  log_i("    _min_heat_setpoint_limit      = %d",              snapshot.min_heat_setpoint_limit      );
  log_i("    _max_heat_setpoint_limit      = %d",              snapshot.max_heat_setpoint_limit      );
  log_i("    _abs_min_heat_setpoint_limit  = %d",              snapshot.abs_min_heat_setpoint_limit  );
  log_i("    _abs_max_heat_setpoint_limit  = %d",              snapshot.abs_max_heat_setpoint_limit  );
  log_i("    _min_cool_setpoint_limit      = %d",              snapshot.min_cool_setpoint_limit      );
  log_i("    _max_cool_setpoint_limit      = %d",              snapshot.max_cool_setpoint_limit      );
  log_i("    _abs_min_cool_setpoint_limit  = %d",              snapshot.abs_min_cool_setpoint_limit  );
  log_i("    _abs_max_cool_setpoint_limit  = %d",              snapshot.abs_max_cool_setpoint_limit  );
  log_i("    _ui_config_display_mode       = %d",              snapshot.ui_config_display_mode       );
  log_i("    _ui_config_keypad_lockout     = %d,  %s",         snapshot.ui_config_keypad_lockout     ,     zb_zcl_thermostat_ui_config_keypad_lockout_to_string((zb_zcl_thermostat_ui_config_keypad_lockout_t)snapshot.ui_config_keypad_lockout));
  log_i("    _stelpro_outdoor_temperature  = %d,  %.1f°C",     snapshot.stelpro_outdoor_temperature  ,     snapshot.stelpro_outdoor_temperature   /100.0    );
  log_i("    _stelpro_system_mode          = %d,  %s",         snapshot.stelpro_system_mode          ,     zb_constants_zcl_thermostat_system_mode_attr_to_string((esp_zb_zcl_thermostat_system_mode_t)snapshot.stelpro_system_mode));
  log_i("    _stelpro_power                = %d W",            snapshot.stelpro_power                );
  log_i("    _stelpro_energy               = %d Wh",           snapshot.stelpro_energy               );
  log_i("    _stelpro_peak_demand_icon     = %d",              snapshot.stelpro_peak_demand_icon     );
}

esp_zb_cluster_list_t * ZigbeeStelproH420Thermostat::zigbee_stelpro_thermostat_clusters_create(zigbee_stelpro_thermostat_cfg_t *stelpro_cfg) {
  esp_err_t err = ESP_OK;

  // Minimum mandatory clusters for a thermostat
  esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_basic_cluster_create(&stelpro_cfg->basic_cfg);
  esp_zb_attribute_list_t *esp_zb_identify_cluster = esp_zb_identify_cluster_create(&stelpro_cfg->identify_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_cluster = esp_zb_thermostat_cluster_create(&stelpro_cfg->thermostat_cfg);

  // Additional cluster configs
  esp_zb_attribute_list_t *esp_zb_groups_cluster = esp_zb_groups_cluster_create(&stelpro_cfg->groups_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_ui_config_cluster = esp_zb_thermostat_ui_config_cluster_create(&stelpro_cfg->thermostat_ui_config_cfg);

  // Thermostat cluster, additional attributes
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _running_state          .getAttributeId(),  _running_state          .getDefaultDataPointer());   logError(err);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _pi_heating_demand      .getAttributeId() , _pi_heating_demand      .getDefaultDataPointer());   logError(err);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _outdoor_temperature    .getAttributeId() , _outdoor_temperature    .getDefaultDataPointer());   logError(err);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _occupancy              .getAttributeId() , _occupancy              .getDefaultDataPointer());   logError(err);

  // Hardcoded HEATING MIN/MAX/ABS SETPOINTS
  #if 1
  {
    // Heating setpoint limits (even for heating-only thermostats)
    static int16_t abs_min_heat_setpoint =  STELPRO_MIN_HEAT_SETPOINT;
    static int16_t abs_max_heat_setpoint =  STELPRO_MAX_HEAT_SETPOINT;
    static int16_t min_heat_setpoint =      STELPRO_MIN_HEAT_SETPOINT;
    static int16_t max_heat_setpoint =      STELPRO_MAX_HEAT_SETPOINT;

    // Add attribute limits
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID, &abs_min_heat_setpoint);   logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID, &abs_max_heat_setpoint);   logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID,     &min_heat_setpoint);       logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID,     &max_heat_setpoint);       logError(err);
  }
  #endif

  // Hardcoded COOLING MIN/MAX/ABS SETPOINTS
  #if 1
  {
    // Cooling setpoint limits (even for heating-only thermostats)
    static int16_t abs_min_cool_setpoint =  STELPRO_MIN_COOL_SETPOINT;
    static int16_t abs_max_cool_setpoint =  STELPRO_MAX_COOL_SETPOINT;
    static int16_t min_cool_setpoint =      STELPRO_MIN_COOL_SETPOINT;
    static int16_t max_cool_setpoint =      STELPRO_MAX_COOL_SETPOINT;

    // Add attribute limits
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID, &abs_min_cool_setpoint);   logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID, &abs_max_cool_setpoint);   logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID,     &min_cool_setpoint);       logError(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID,     &max_cool_setpoint);       logError(err);
  }
  #endif

  // Manufacturer attributes variables
  #if 1
  {
    // StelproOutdoorTemp:
    // Note:
    // When adding the attribute with function esp_zb_cluster_add_manufacturer_attr(), zigbee2mqtt is unable to read or write the attribute.
    // The following error is reported:
    // ```
    // [2/26/2026, 5:26:10 PM] z2m: Publish 'set' 'write' to '0x123abcdefa123456' failed: 'Error: ZCL command 0x123abcdefa123456/25 hvacThermostat.write(
    // {"StelproOutdoorTemp":-10}, {"timeout":10000,"disableResponse":false,"disableRecovery":false,"disableDefaultResponse":true,"direction":0,"reservedBits":0,"writeUndiv":false})
    // failed (Status 'UNSUPPORTED_ATTRIBUTE')'
    // ```
    // Note that it could also be a bug in ESP ZIGBEE SDK's library which do not register attribute with custom manufacturer codes.
    // The current solution/workaround is to add the attribute as a normal attribute (without a custom manufacterer code).
    // Use `esp_zb_cluster_add_attr()` instead of `esp_zb_cluster_add_manufacturer_attr()`.
    err = esp_zb_cluster_add_attr(
      esp_zb_thermostat_cluster,
      _stelpro_outdoor_temperature.getClusterId(),
      _stelpro_outdoor_temperature.getAttributeId(),
      ESP_ZB_ZCL_ATTR_TYPE_S16,
      ESP_ZB_ZCL_ATTR_ACCESS_REPORTING | ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE,
      _stelpro_outdoor_temperature.getDefaultDataPointer()
    );
    logError(err);
  }
  #endif

  #if 1
  {
    // StelproSystemMode:
    // Create a clone of the existing thermostat's SystemMode attribute.
    // Maintain synchronisation of both attributes explicitly in `zbAttributeSet()` and `setStelproSystemMode()` via the helper method `updateSystemModes()`.
    // Whenever either attribute is written, the other is immediately updated to the same value.

    // Find existing SystemMode attribute
    esp_zb_zcl_attr_t * system_mode_attr = zb_find_attribute_in_attribute_list(esp_zb_thermostat_cluster, _system_mode.getAttributeId());
    if (system_mode_attr == nullptr) {
      log_e("Failed to find attribute: '%s'", _system_mode.getName());
    } else {
      // Create new StelproSystemMode attribute using system_mode's data pointer.
      // The attribute won't use the same data pointer as system_mode attribute but it will effectively copy the default starting value.
      err = esp_zb_cluster_add_attr(
        esp_zb_thermostat_cluster,
        _stelpro_system_mode.getClusterId(),
        _stelpro_system_mode.getAttributeId(),
        system_mode_attr->type,
        system_mode_attr->access,
        system_mode_attr->data_p
      );
      logError(err);

      if (err == ESP_OK) {
        esp_zb_zcl_attr_t * stelpro_system_mode_attr = zb_find_attribute_in_attribute_list(esp_zb_thermostat_cluster, _stelpro_system_mode.getAttributeId());
        if (stelpro_system_mode_attr == nullptr) {
          log_e("*** Failed to find attribute: '%s'", _stelpro_system_mode.getName());
        }
      }
    }
  }
  #endif

  #if 1
  {
    // StelproPower :
    err = esp_zb_cluster_add_attr(
      esp_zb_thermostat_cluster,
      _stelpro_power.getClusterId(),
      _stelpro_power.getAttributeId(),
      ESP_ZB_ZCL_ATTR_TYPE_U16,
      ESP_ZB_ZCL_ATTR_ACCESS_REPORTING | ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
      _stelpro_power.getDefaultDataPointer()
    );
    logError(err);

    // StelproEnergy :
    err = esp_zb_cluster_add_attr(
      esp_zb_thermostat_cluster,
      _stelpro_energy.getClusterId(),
      _stelpro_energy.getAttributeId(),
      ESP_ZB_ZCL_ATTR_TYPE_U32,
      ESP_ZB_ZCL_ATTR_ACCESS_REPORTING | ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
      _stelpro_energy.getDefaultDataPointer()
    );
    logError(err);
  }
  #endif

  #if 1
  {
    // StelproPeakDemandIcon :
    err = esp_zb_cluster_add_attr(
      esp_zb_thermostat_cluster,
      _stelpro_peak_demand_icon.getClusterId(),
      _stelpro_peak_demand_icon.getAttributeId(),
      ESP_ZB_ZCL_ATTR_TYPE_U16,
      ESP_ZB_ZCL_ATTR_ACCESS_REPORTING | ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE,
      _stelpro_peak_demand_icon.getDefaultDataPointer()
    );
    logError(err);
  }
  #endif

  // Create cluster list
  esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();
  esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list,                esp_zb_basic_cluster,                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list,             esp_zb_identify_cluster,              ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_groups_cluster(esp_zb_cluster_list,               esp_zb_groups_cluster,                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_cluster(esp_zb_cluster_list,           esp_zb_thermostat_cluster,            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_ui_config_cluster(esp_zb_cluster_list, esp_zb_thermostat_ui_config_cluster,  ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  return esp_zb_cluster_list;
}
