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

  logEntry( "Unexpected data type received: cluster=%s, attr=%s, expected=%s, actual=%s", cluster_name, attr_name, expected_name, actual_name);
}

void logUnhandledMessageError(uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, uint16_t type_id) {
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;

  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  logEntry("Received unknown message: endpoint=%d, cluster=%s (0x%04x), attr=%s (0x%04x), type=%s (0x%04x)",
    endpoint, cluster_name, cluster_id, attr_name, attr_id, zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)type_id), type_id );
}

ZigbeeStelproH420Thermostat::ZigbeeStelproH420Thermostat(uint8_t endpoint) : ZigbeeEP(endpoint) {
#define ENABLE_SERIAL_DEBUGGING_IN_CTOR
#ifdef ENABLE_SERIAL_DEBUGGING_IN_CTOR
  // HACK for debugging logEntry() calls since ZigbeeStelproH420Thermostat as a static object before setup() is called.
  // Wait up to 3s for serial
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif // #ifdef ENABLE_SERIAL_DEBUGGING_IN_CTOR

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
  _stelpro_outdoor_temperature                .init("_stelpro_outdoor_temperature"           , STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID);

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

  // Assert all attributes are initialized
  {
    for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
      IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
      if (!attr_p->isInitialized()) {
        logEntry("WARNING: Attribute [%d] is not initialized: %s", i, attr_p->toString().c_str());
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
  zb_debug_print_attributes_in_cluster_list(_cluster_list);

  // Set zigbee default attribute initialization values.
  // The following zigbee attributes are attributes automatically created by esp_zb_thermostat_cluster_create() and other similar functions.
  // We need to initialize their default value before they are created in the zigbee stack.
  if (!zb_set_attribute_value_in_cluster_list<uint16_t>   (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID            , THERMOSTAT_RUNNING_STATE_IDLE))                                             logEntry("Failed to set attribute 'RUNNING_STATE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID                   , 0))                                                                         logEntry("Failed to set attribute 'PI_HEATING_DEMAND' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<int16_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID                 , 0 ))                                                                        logEntry("Failed to set attribute 'OUTDOOR_TEMPERATURE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<zb_uint8_t> (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID                           , ESP_ZB_ZCL_THERMOSTAT_OCCUPANCY_DEFAULT_VALUE ))                            logEntry("Failed to set attribute 'OCCUPANCY' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG  , ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID  , ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE ))   logEntry("Failed to set attribute 'DISPLAY_MODE' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<uint8_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG  , ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID            , ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_DEFAULT_VALUE ))             logEntry("Failed to set attribute 'KEYPAD_LOCKOUT' value in cluster list!");
  if (!zb_set_attribute_value_in_cluster_list<int16_t>    (_cluster_list, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT            , ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID                    , 0 ))                                                                        logEntry("Failed to set attribute 'STELPRO_OUTDOOR_TEMPERATURE' value in cluster list!");
  
  // Set endpoint configuration
  _ep_config = {
    .endpoint = _endpoint,
    .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
    .app_device_id = ESP_ZB_HA_THERMOSTAT_DEVICE_ID,
    .app_device_version = 0
  };

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Initialize metering values
  _instantaneous_demand = {.low = 0, .high = 0};
  _current_summation_delivered = {.low = 0, .high = 0};
  _multiplier = {.low = 1, .high = 0};
  _divisor = {.low = 1000, .high = 0};
  _unit_of_measure = 0x00;  // kWh/kW
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS
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

  // Check if value has changed. Ignore the attribute set command if not changed.
  if (!attr->hasChanged()) {
    return;
  }

  // Notify observers of the change
  attr->notifyChange();
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

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
bool ZigbeeStelproH420Thermostat::setInstantaneousDemand(int32_t demand_watts) {
  // Convert to demand with divisor applied (watts)
  _instantaneous_demand.low = demand_watts & 0xFFFF;
  _instantaneous_demand.high = (demand_watts >> 16) & 0xFF;

  esp_zb_lock_acquire(portMAX_DELAY);
  esp_zb_zcl_status_t status = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_METERING_INSTANTANEOUS_DEMAND_ID,
    &_instantaneous_demand,
    false
  );
  esp_zb_lock_release();

  if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set instantaneous demand: 0x%x", status);
    return false;
  }
  return true;
}

bool ZigbeeStelproH420Thermostat::setCurrentSummationDelivered(uint64_t energy_wh) {
  // Store energy in Wh
  _current_summation_delivered.low = energy_wh & 0xFFFFFFFF;
  _current_summation_delivered.high = (energy_wh >> 32) & 0xFFFF;

  esp_zb_lock_acquire(portMAX_DELAY);
  esp_zb_zcl_status_t status = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID,
    &_current_summation_delivered,
    false
  );
  esp_zb_lock_release();

  if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set current summation delivered: 0x%x", status);
    return false;
  }
  return true;
}
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

void ZigbeeStelproH420Thermostat::updateEnergy() {
#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Update energy calculator
  _energy_calc.loop();
  
  // Get calculated values from energy calculator
  uint8_t pi_heating_demand = _energy_calc.getPIHeatingDemand();
  uint16_t running_state = _energy_calc.getRunningState();
  int32_t power_watts = _energy_calc.getInstantaneousDemandWatts();
  uint64_t energy_wh = _energy_calc.getCurrentSummationDelivered();
  
  // Update Zigbee attributes with calculated values
  setPIHeatingDemand(pi_heating_demand);
  setRunningState(running_state);
  setInstantaneousDemand(power_watts);
  setCurrentSummationDelivered(energy_wh);
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS
}

bool ZigbeeStelproH420Thermostat::update() {
  // Remember previous values
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
    if (!attr_p->update()) {
      logEntry("WARNING: Attribute [%d] is failed to update: %s", i, attr_p->toString().c_str());
    }
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
      logEntry("WARNING: Attribute [%d] has failed to setup(): %s", i, attr_p->toString().c_str());
    }
    success = success && isReady;
  }

  if (!success) {
    const char * error_msg1 = "************************************************************************************";
    const char * error_msg2 = "*                                                                                  *";
    const char * error_msg3 = "*                   Some zigbee attributes has failed to setup!                    *";
    logEntry(error_msg1);
    logEntry(error_msg2);
    logEntry(error_msg2);
    logEntry(error_msg3);
    logEntry(error_msg2);
    logEntry(error_msg2);
    logEntry(error_msg1);
  }

  logEntry("attributes: {");
  for(size_t i=0; i<_zigbee_attribute_list.size(); i++) {
    IZigbeeAttribute* attr_p = _zigbee_attribute_list[i];
    logEntry("    %s", attr_p->toString().c_str());
  }
  logEntry("}");
  return success;
}

void ZigbeeStelproH420Thermostat::printZigbeeAttributes() {
  logEntry("-----> _local_temperature            .get()=%d", _local_temperature            .get());
  logEntry("-----> _occupied_cooling_setpoint    .get()=%d", _occupied_cooling_setpoint    .get());
  logEntry("-----> _occupied_heating_setpoint    .get()=%d", _occupied_heating_setpoint    .get());
  logEntry("-----> _control_sequence_of_operation.get()=%d", _control_sequence_of_operation.get());
  logEntry("-----> _system_mode                  .get()=%d", _system_mode                  .get());
  logEntry("-----> _running_state                .get()=%d", _running_state                .get());
  logEntry("-----> _pi_heating_demand            .get()=%d", _pi_heating_demand            .get());
  logEntry("-----> _outdoor_temperature          .get()=%d", _outdoor_temperature          .get());
  logEntry("-----> _occupancy                    .get()=%d", _occupancy                    .get());
  logEntry("-----> _min_heat_setpoint_limit      .get()=%d", _min_heat_setpoint_limit      .get());
  logEntry("-----> _max_heat_setpoint_limit      .get()=%d", _max_heat_setpoint_limit      .get());
  logEntry("-----> _abs_min_heat_setpoint_limit  .get()=%d", _abs_min_heat_setpoint_limit  .get());
  logEntry("-----> _abs_max_heat_setpoint_limit  .get()=%d", _abs_max_heat_setpoint_limit  .get());
  logEntry("-----> _min_cool_setpoint_limit      .get()=%d", _min_cool_setpoint_limit      .get());
  logEntry("-----> _max_cool_setpoint_limit      .get()=%d", _max_cool_setpoint_limit      .get());
  logEntry("-----> _abs_min_cool_setpoint_limit  .get()=%d", _abs_min_cool_setpoint_limit  .get());
  logEntry("-----> _abs_max_cool_setpoint_limit  .get()=%d", _abs_max_cool_setpoint_limit  .get());
  logEntry("-----> _ui_config_display_mode       .get()=%d", _ui_config_display_mode       .get());
  logEntry("-----> _ui_config_keypad_lockout     .get()=%d", _ui_config_keypad_lockout     .get());
  logEntry("-----> _stelpro_outdoor_temperature  .get()=%d", _stelpro_outdoor_temperature  .get());
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
    if (!attribute.getUnsafeCopy( output )) {                                                     \
      logEntry("WARNING: Failed to get attribute in snapshot: %s", attribute.toString().c_str()); \
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

  #undef CAPTURE_ATTR

  esp_zb_lock_release();

  // Assert that all snapshot variables were captured
  // (assert total size of all attributes matches the size of the snapshot struct)
  size_t snapshot_size = sizeof(snapshot);
  if (success && capture_size != snapshot_size) {
    logEntry("WARNING: Failed to get all attributes in snapshot: snapshot_size=%d, capture_size=%d", snapshot_size, capture_size);
    return false;
  }

  return success;
}

void ZigbeeStelproH420Thermostat::printSnapshot(const zb_zcl_stelpro_thermostat_snapshot_t& snapshot) {
  logEntry("    _local_temperature            = %d,  %.1f°C",     snapshot.local_temperature            ,     snapshot.local_temperature        /100.0    );
  logEntry("    _occupied_cooling_setpoint    = %d,  %.1f°C",     snapshot.occupied_cooling_setpoint    ,     snapshot.occupied_cooling_setpoint/100.0    );
  logEntry("    _occupied_heating_setpoint    = %d,  %.1f°C",     snapshot.occupied_heating_setpoint    ,     snapshot.occupied_heating_setpoint/100.0    );
  logEntry("    _control_sequence_of_operation= %d",              snapshot.control_sequence_of_operation);
  logEntry("    _system_mode                  = %d,  %s",         snapshot.system_mode                  ,     zb_constants_zcl_thermostat_system_mode_attr_to_string((esp_zb_zcl_thermostat_system_mode_t)snapshot.system_mode));
  logEntry("    _running_state                = %d,  %s",         snapshot.running_state                ,     zb_constants_zcl_thermostat_running_state_attr_to_string(snapshot.running_state).c_str());
  logEntry("    _pi_heating_demand            = %d%%",            snapshot.pi_heating_demand            );
  logEntry("    _outdoor_temperature          = %d,  %.1f°C",     snapshot.outdoor_temperature          ,     snapshot.outdoor_temperature      /100.0    );
  logEntry("    _occupancy                    = %d",              snapshot.occupancy                    );
  logEntry("    _min_heat_setpoint_limit      = %d",              snapshot.min_heat_setpoint_limit      );
  logEntry("    _max_heat_setpoint_limit      = %d",              snapshot.max_heat_setpoint_limit      );
  logEntry("    _abs_min_heat_setpoint_limit  = %d",              snapshot.abs_min_heat_setpoint_limit  );
  logEntry("    _abs_max_heat_setpoint_limit  = %d",              snapshot.abs_max_heat_setpoint_limit  );
  logEntry("    _min_cool_setpoint_limit      = %d",              snapshot.min_cool_setpoint_limit      );
  logEntry("    _max_cool_setpoint_limit      = %d",              snapshot.max_cool_setpoint_limit      );
  logEntry("    _abs_min_cool_setpoint_limit  = %d",              snapshot.abs_min_cool_setpoint_limit  );
  logEntry("    _abs_max_cool_setpoint_limit  = %d",              snapshot.abs_max_cool_setpoint_limit  );
  logEntry("    _ui_config_display_mode       = %d",              snapshot.ui_config_display_mode       );
  logEntry("    _ui_config_keypad_lockout     = %d,  %s",         snapshot.ui_config_keypad_lockout     ,     zb_zcl_thermostat_ui_config_keypad_lockout_to_string((zb_zcl_thermostat_ui_config_keypad_lockout_t)snapshot.ui_config_keypad_lockout));
  logEntry("    _stelpro_outdoor_temperature  = %d,  %.1f°C",     snapshot.stelpro_outdoor_temperature  ,     snapshot.stelpro_outdoor_temperature   /100.0    );
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
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _running_state          .getAttributeId(),  _running_state          .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _pi_heating_demand      .getAttributeId() , _pi_heating_demand      .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _outdoor_temperature    .getAttributeId() , _outdoor_temperature    .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _occupancy              .getAttributeId() , _occupancy              .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);

  // Hardcoded HEATING MIN/MAX/ABS SETPOINTS
  #if 1
  {
    // Heating setpoint limits (even for heating-only thermostats)
    static int16_t abs_min_heat_setpoint =  STELPRO_MIN_HEAT_SETPOINT;
    static int16_t abs_max_heat_setpoint =  STELPRO_MAX_HEAT_SETPOINT;
    static int16_t min_heat_setpoint =      STELPRO_MIN_HEAT_SETPOINT;
    static int16_t max_heat_setpoint =      STELPRO_MAX_HEAT_SETPOINT;

    // Add attribute limits
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID, &abs_min_heat_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID, &abs_max_heat_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID,     &min_heat_setpoint);       ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID,     &max_heat_setpoint);       ZB_LOG_ERROR(err);
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
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID, &abs_min_cool_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID, &abs_max_cool_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID,     &min_cool_setpoint);       ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID,     &max_cool_setpoint);       ZB_LOG_ERROR(err);
  }
  #endif

  // Manufacturer attributes variables

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
    ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE,
    _stelpro_outdoor_temperature.getDefaultDataPointer()
  );
  logError(err, __FILE__, __LINE__);

#ifdef ENABLE_STELPRO_POWER_MEASUREMENTS
  // Add control sequence (HEATING ONLY)
  //uint8_t control_sequence = 0x02;  // Heating only
  esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID, &control_sequence);
  
  // Create Simple Metering cluster for power/energy reporting
  esp_zb_attribute_list_t *esp_zb_metering_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_METERING);
  
  esp_zb_cluster_add_attr(
    esp_zb_metering_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID,
    ESP_ZB_ZCL_ATTR_TYPE_U48,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING,
    &_current_summation_delivered
  );
  
  esp_zb_cluster_add_attr(
    esp_zb_metering_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_ATTR_METERING_INSTANTANEOUS_DEMAND_ID,
    ESP_ZB_ZCL_ATTR_TYPE_S24,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING,
    &_instantaneous_demand
  );
  
  esp_zb_cluster_add_attr(
    esp_zb_metering_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_ATTR_METERING_UNIT_OF_MEASURE_ID,
    ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
    &_unit_of_measure
  );
  
  esp_zb_cluster_add_attr(
    esp_zb_metering_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_ATTR_METERING_MULTIPLIER_ID,
    ESP_ZB_ZCL_ATTR_TYPE_U24,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
    &_multiplier
  );
  
  esp_zb_cluster_add_attr(
    esp_zb_metering_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_ATTR_METERING_DIVISOR_ID,
    ESP_ZB_ZCL_ATTR_TYPE_U24,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
    &_divisor
  );
#endif // ENABLE_STELPRO_POWER_MEASUREMENTS

  // Create cluster list
  esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();
  esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list,                esp_zb_basic_cluster,                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list,             esp_zb_identify_cluster,              ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_groups_cluster(esp_zb_cluster_list,               esp_zb_groups_cluster,                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_cluster(esp_zb_cluster_list,           esp_zb_thermostat_cluster,            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_ui_config_cluster(esp_zb_cluster_list, esp_zb_thermostat_ui_config_cluster,  ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  return esp_zb_cluster_list;
}
