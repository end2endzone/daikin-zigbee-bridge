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
  
  // Initialize default values
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  _stelpro_outdoor_temp = 0;
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

#ifdef USE_ZB_CLASSES
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
#else // USE_ZB_CLASSES
#endif // USE_ZB_CLASSES

  // Build cluster lists
  zigbee_stelpro_thermostat_cfg_t thermostat_cfg = ZIGBEE_DEFAULT_STELPRO_THERMOSTAT_CONFIG();
  _cluster_list = zigbee_stelpro_thermostat_clusters_create(&thermostat_cfg);

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
  
#ifdef USE_ZB_CLASSES
  if (_local_temperature.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
      return;
    }
    if (callbacks._on_local_temperature_change) {
      callbacks._on_local_temperature_change(_local_temperature.get());
    }
  }
  else if (_occupied_cooling_setpoint.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
      return;
    }
    if (callbacks._on_occupied_cool_setpoint_change) {
      callbacks._on_occupied_cool_setpoint_change(_occupied_cooling_setpoint.get());
    }
  }
  else if (_occupied_heating_setpoint.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
      return;
    }
    if (callbacks._on_occupied_heat_setpoint_change) {
      callbacks._on_occupied_heat_setpoint_change(_occupied_heating_setpoint.get());
    }
  }
  else if (_control_sequence_of_operation.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
      return;
    }
    if (callbacks._on_control_sequence_of_operation_change) {
      callbacks._on_control_sequence_of_operation_change(_control_sequence_of_operation.get());
    }
  }
  else if (_system_mode.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
      return;
    }
    if (callbacks._on_system_mode_change) {
      callbacks._on_system_mode_change(_system_mode.get());
    }
  }
  else if (_running_state.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U16) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U16, message->attribute.id);
      return;
    }
    if (callbacks._on_running_state_change) {
      callbacks._on_running_state_change(_running_state.get());
    }
  }
  else if (_pi_heating_demand.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
      return;
    }
    if (callbacks._on_pi_heating_demand_change) {
      callbacks._on_pi_heating_demand_change(_pi_heating_demand.get());
    }
  }
  else if (_outdoor_temperature.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
      return;
    }
    if (callbacks._on_outdoor_temperature_change) {
      callbacks._on_outdoor_temperature_change(_outdoor_temperature.get());
    }
  }
  else if (_occupancy.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
      return;
    }
    if (callbacks._on_occupancy_change) {
      callbacks._on_occupancy_change(_occupancy.get());
    }
  }
  else if (_ui_config_display_mode.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
      return;
    }
    if (callbacks._on_ui_config_display_mode_change) {
      callbacks._on_ui_config_display_mode_change(_ui_config_display_mode.get());
    }
  }
  else if (_ui_config_keypad_lockout.matches(message->info.dst_endpoint, message->info.cluster, message->attribute.id)) {
    if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
      // ERROR
      logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
      return;
    }
    if (callbacks._on_ui_config_keypad_lockout_change) {
      callbacks._on_ui_config_keypad_lockout_change(_ui_config_keypad_lockout.get());
    }
  }
  else {
    logUnhandledMessageError(message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.type);
  }

#else // USE_ZB_CLASSES
  if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT) {
    // Thermostat cluster
    switch (message->attribute.id) {
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }
        int16_t new_temperature = *(int16_t *)message->attribute.data.value;
        int16_t old_temperature = 0;
        bool success = getLocalTemperature(old_temperature);
        if (old_temperature != new_temperature) {
          if (callbacks._on_local_temperature_change) {
            callbacks._on_local_temperature_change(new_temperature);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }
        int16_t new_setpoint = *(int16_t *)message->attribute.data.value;
        int16_t old_setpoint = 0;
        bool success = getOccupiedCoolingSetpoint(old_setpoint);
        if (old_setpoint != new_setpoint) {
          if (callbacks._on_occupied_cool_setpoint_change) {
            callbacks._on_occupied_cool_setpoint_change(new_setpoint);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }
        int16_t new_setpoint = *(int16_t *)message->attribute.data.value;
        int16_t old_setpoint = 0;
        bool success = getOccupiedHeatingSetpoint(old_setpoint);
        if (old_setpoint != new_setpoint) {
          if (callbacks._on_occupied_heat_setpoint_change) {
            callbacks._on_occupied_heat_setpoint_change(new_setpoint);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
          return;
        }
        uint8_t new_mode = *(uint8_t *)message->attribute.data.value;
        uint8_t old_mode = 0;
        bool success = getSystemMode(old_mode);
        if (old_mode != new_mode) {
          if (callbacks._on_system_mode_change) {
            callbacks._on_system_mode_change(new_mode);
          }
        }
        break;
      }

      // Thermostat cluster, additional attributes
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U16, message->attribute.id);
          return;
        }
        uint16_t new_running_state = *(uint16_t *)message->attribute.data.value;
        uint16_t old_running_state = 0;
        bool success = getRunningState(old_running_state);
        if (old_running_state != new_running_state) {
          if (callbacks._on_running_state_change) {
            callbacks._on_running_state_change(new_running_state);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
          return;
        }
        uint8_t new_pi_heating_demand = *(uint8_t *)message->attribute.data.value;
        uint8_t old_pi_heating_demand = 0;
        bool success = getPIHeatingDemand(old_pi_heating_demand);
        if (old_pi_heating_demand != new_pi_heating_demand) {
          if (callbacks._on_pi_heating_demand_change) {
            callbacks._on_pi_heating_demand_change(new_pi_heating_demand);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }
        int16_t new_outdoor_temperature = *(int16_t *)message->attribute.data.value;
        int16_t old_outdoor_temperature = 0;
        bool success = getOutdoorTemperature(old_outdoor_temperature);
        if (old_outdoor_temperature != new_outdoor_temperature) {
          if (callbacks._on_outdoor_temperature_change) {
            callbacks._on_outdoor_temperature_change(new_outdoor_temperature);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U16, message->attribute.id);
          return;
        }
        uint8_t new_occupancy = *(uint8_t *)message->attribute.data.value;
        uint8_t old_occupancy = 0;
        bool success = getOccupancy(old_occupancy);
        if (old_occupancy != new_occupancy) {
          if (callbacks._on_occupancy_change) {
            callbacks._on_occupancy_change(new_occupancy);
          }
        }
        break;
      }

#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
      case ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }

        int16_t new_stelpro_outdoor_temp = *(int16_t *)message->attribute.data.value;
        if (_stelpro_outdoor_temp != new_stelpro_outdoor_temp) {
          _stelpro_outdoor_temp = new_stelpro_outdoor_temp;
          if (_on_stelpro_outdoor_temp_change) {
            _on_stelpro_outdoor_temp_change(_stelpro_outdoor_temp);
          }
        }
        break;
      }
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

      default:
        logUnhandledMessageError(message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.type);
        break;
    }
  }
  else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG) {
    // Thermostat UI cluster
    switch (message->attribute.id) {
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
          return;
        }
        uint8_t new_display_mode = *(uint8_t *)message->attribute.data.value;
        uint8_t old_display_mode = 0;
        bool success = getSystemMode(old_display_mode);
        if (old_display_mode != new_display_mode) {
          if (callbacks._on_ui_config_display_mode_change) {
            callbacks._on_ui_config_display_mode_change(new_display_mode);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
          // ERROR
          logUnexpectedDataTypeReceived(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
          return;
        }
        uint8_t new_keypad_lockout = *(uint8_t *)message->attribute.data.value;
        uint8_t old_keypad_lockout = 0;
        bool success = getKeypadLockout(old_keypad_lockout);
        if (old_keypad_lockout != new_keypad_lockout) {
          if (callbacks._on_ui_config_keypad_lockout_change) {
            callbacks._on_ui_config_keypad_lockout_change(new_keypad_lockout);
          }
        }
        break;
      }

      default:
        logUnhandledMessageError(message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.type);
        break;
    };
  }
  else {
    // ERROR
    logUnhandledMessageError(message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.type);
  }
#endif // USE_ZB_CLASSES

}

// Zigbee attribute setters
// Thermostat cluster mandatory attributes
bool ZigbeeStelproH420Thermostat::setLocalTemperature(int16_t temperature) {
#ifdef USE_ZB_CLASSES
  bool success = _local_temperature.set(temperature);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID, temperature);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_local_temperature_change) {
    callbacks._on_local_temperature_change(temperature);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedCoolingSetpoint(int16_t setpoint) {
#ifdef USE_ZB_CLASSES
  bool success = _occupied_cooling_setpoint.set(setpoint);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID, setpoint);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_occupied_cool_setpoint_change) {
    callbacks._on_occupied_cool_setpoint_change(setpoint);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedHeatingSetpoint(int16_t setpoint) {
#ifdef USE_ZB_CLASSES
  bool success = _occupied_heating_setpoint.set(setpoint);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID, setpoint);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_occupied_heat_setpoint_change) {
    callbacks._on_occupied_heat_setpoint_change(setpoint);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setControlSequenceOfOperation(uint8_t csop) {
#ifdef USE_ZB_CLASSES
  bool success = _control_sequence_of_operation.set(csop);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION_ID, csop);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_control_sequence_of_operation_change) {
    callbacks._on_control_sequence_of_operation_change(csop);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setSystemMode(uint8_t mode) {
#ifdef USE_ZB_CLASSES
  bool success = _system_mode.set(mode);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID, mode);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_system_mode_change) {
    callbacks._on_system_mode_change(mode);
  }
  return success;
}

// Thermostat cluster, additional attributes
bool ZigbeeStelproH420Thermostat::setRunningState(uint16_t state) {
#ifdef USE_ZB_CLASSES
  bool success = _running_state.set(state);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID, state);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_running_state_change) {
    callbacks._on_running_state_change(state);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setPIHeatingDemand(uint8_t demand) {
#ifdef USE_ZB_CLASSES
  bool success = _pi_heating_demand.set(demand);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID, demand);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_pi_heating_demand_change) {
    callbacks._on_pi_heating_demand_change(demand);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOutdoorTemperature(int16_t temperature) {
#ifdef USE_ZB_CLASSES
  bool success = _outdoor_temperature.set(temperature);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID, temperature);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_outdoor_temperature_change) {
    callbacks._on_outdoor_temperature_change(temperature);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupancy(zb_uint8_t occupancy) {
#ifdef USE_ZB_CLASSES
  bool success = _occupancy.set(occupancy);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID, occupancy);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_occupancy_change) {
    callbacks._on_occupancy_change(occupancy);
  }
  return success;
}

// Thermostat UI cluster
bool ZigbeeStelproH420Thermostat::setTemperatureDisplayMode(uint8_t mode) {
#ifdef USE_ZB_CLASSES
  bool success = _ui_config_display_mode.set(mode);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID, mode);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_ui_config_display_mode_change) {
    callbacks._on_ui_config_display_mode_change(mode);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setKeypadLockout(uint8_t lockout) {
#ifdef USE_ZB_CLASSES
  bool success = _ui_config_keypad_lockout.set(lockout);
#else // USE_ZB_CLASSES
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID, lockout);
#endif // USE_ZB_CLASSES
  if (!success)
    return false;

  if (callbacks._on_ui_config_keypad_lockout_change) {
    callbacks._on_ui_config_keypad_lockout_change(lockout);
  }
  return success;
}

#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
bool ZigbeeStelproH420Thermostat::setStelproOutdoorTemp(int16_t temperature) {
  esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

  // logEntry("Updating stelpro outdoor temperature to %d", temperature);

  // Update cluster
  esp_zb_lock_acquire(portMAX_DELAY);
  // set attribute value
  ret = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID,
    &_stelpro_outdoor_temp,
    false
  );
  if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set stelpro outdoor temperature. Status: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
    goto unlock_and_return;
  }
  
  // Update attribute
  _stelpro_outdoor_temp = temperature;
  if (_on_stelpro_outdoor_temp_change) {
    _on_stelpro_outdoor_temp_change(temperature);
  }
unlock_and_return:
  esp_zb_lock_release();
  return ret == ESP_ZB_ZCL_STATUS_SUCCESS;
}
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

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

bool ZigbeeStelproH420Thermostat::report() {
  bool success = true;
#ifdef USE_ZB_CLASSES
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
#else // USE_ZB_CLASSES
#endif // USE_ZB_CLASSES
  return success;
}

bool ZigbeeStelproH420Thermostat::setup() {
#ifdef USE_ZB_CLASSES
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

  return success;

#else // USE_ZB_CLASSES
  return true;
#endif // USE_ZB_CLASSES
}


#ifdef USE_ZB_CLASSES
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
}
#else // USE_ZB_CLASSES
bool ZigbeeStelproH420Thermostat::getGenericAttribute(uint16_t cluster_id, uint16_t attr_id, void* output_ptr, size_t output_size) const {
  if (output_ptr == nullptr || output_size == 0)
    return false;

  bool ret = true;
  size_t attr_size = 0;
  
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;
  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  // logEntry("Reading attribute 0x%04x (%s) of cluster 0x%04x (%s)", attr_id, attr_name, cluster_id, cluster_name);

  esp_zb_lock_acquire(portMAX_DELAY);
  
  // get attribute descriptor
  esp_zb_zcl_attr_t * attr = esp_zb_zcl_get_attribute(
    _endpoint,
    cluster_id,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    attr_id);
  if (attr == nullptr) {
    logEntry("Failed to read attribute 0x%04x (%s) of cluster 0x%04x (%s)", attr_id, attr_name, cluster_id, cluster_name);
    ret = false;
    goto unlock_and_return;
  }

  // assert attr size
  attr_size = zb_constants_zcl_attr_type_size((esp_zb_zcl_attr_type_t)attr->type);
  if (attr_size != output_size) {
    logEntry( "Assertion failed to read attribute 0x%04x (%s) of cluster 0x%04x (%s): output value size (%d bytes) does not match registered attribute size (%d bytes). "
              "Attribute type is registered as %s.", attr_id, attr_name, cluster_id, cluster_name, output_size, attr_size, zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)attr->type));
    ret = false;
    goto unlock_and_return;
  }

  // Read the value
  memcpy(output_ptr, attr->data_p, output_size);

unlock_and_return:
  esp_zb_lock_release();
  return ret;
}

bool ZigbeeStelproH420Thermostat::setGenericAttribute(uint16_t cluster_id, uint16_t attr_id, const void* value_ptr, size_t value_size) const {
  if (value_ptr == nullptr || value_size == 0)
    return false;

  bool ret = true;
  size_t attr_size = 0;
  esp_zb_zcl_status_t status = ESP_ZB_ZCL_STATUS_SUCCESS;
  
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;
  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  // logEntry("Writing attribute 0x%04x (%s) of cluster 0x%04x (%s)", attr_id, attr_name, cluster_id, cluster_name);

  esp_zb_lock_acquire(portMAX_DELAY);
  
  // get attribute descriptor
  esp_zb_zcl_attr_t * attr = esp_zb_zcl_get_attribute(
    _endpoint,
    cluster_id,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    attr_id);
  if (attr == nullptr) {
    logEntry("Failed to get attribute 0x%04x (%s) of cluster 0x%04x (%s)", attr_id, attr_name, cluster_id, cluster_name);
    ret = false;
    goto unlock_and_return;
  }

  // assert attr size
  attr_size = zb_constants_zcl_attr_type_size((esp_zb_zcl_attr_type_t)attr->type);
  if (attr_size != value_size) {
    logEntry( "Assertion failed to write attribute 0x%04x (%s) of cluster 0x%04x (%s): new value size (%d bytes) does not match registered attribute size (%d bytes). "
              "Attribute type is registered as %s.", attr_id, attr_name, cluster_id, cluster_name, value_size, attr_size, zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)attr->type));
    ret = false;
    goto unlock_and_return;
  }

  // set attribute value
  status = esp_zb_zcl_set_attribute_val(
    _endpoint,
    cluster_id,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    attr_id,
    (void*)value_ptr,
    false
  );
  if (status != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to write attribute 0x%04x (%s) of cluster 0x%04x (%s). Status: 0x%x: %s", attr_id, attr_name, cluster_id, cluster_name, status, esp_zb_zcl_status_to_name(status));
    ret = false;
    goto unlock_and_return;
  }

unlock_and_return:
  esp_zb_lock_release();
  return ret;
}
#endif // USE_ZB_CLASSES

esp_zb_cluster_list_t * ZigbeeStelproH420Thermostat::zigbee_stelpro_thermostat_clusters_create(zigbee_stelpro_thermostat_cfg_t *thermostat_cfg) {
  esp_err_t err = ESP_OK;

  // Create clusters from device config with mandatory attributes
  esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_basic_cluster_create(&thermostat_cfg->basic_cfg);
  esp_zb_attribute_list_t *esp_zb_identify_cluster = esp_zb_identify_cluster_create(&thermostat_cfg->identify_cfg);
  esp_zb_attribute_list_t *esp_zb_groups_cluster = esp_zb_groups_cluster_create(&thermostat_cfg->groups_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_cluster = esp_zb_thermostat_cluster_create(&thermostat_cfg->thermostat_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_ui_config_cluster = esp_zb_thermostat_ui_config_cluster_create(&thermostat_cfg->thermostat_ui_config_cfg);

  // Thermostat cluster, additional attributes
#ifdef USE_ZB_CLASSES
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _running_state          .getAttributeId(),  _running_state          .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _pi_heating_demand      .getAttributeId() , _pi_heating_demand      .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _outdoor_temperature    .getAttributeId() , _outdoor_temperature    .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, _occupancy              .getAttributeId() , _occupancy              .getDefaultDataPointer());   logError(err, __FILE__, __LINE__);
#else // USE_ZB_CLASSES
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID, &_running_state);    logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID, &_pi_heating_demand);       logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID, &_outdoor_temperature);   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID, &_occupancy);                       logError(err, __FILE__, __LINE__);
#endif // USE_ZB_CLASSES

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

#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  // Add Stelpro custom outdoor temp (0x4001)
  esp_zb_cluster_add_attr(
    esp_zb_thermostat_cluster,
    ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID,
    ESP_ZB_ZCL_ATTR_TYPE_S16,
    ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE,
    &_stelpro_outdoor_temp
  );
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP

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
