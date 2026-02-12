#include "ZigbeeStelproH420Thermostat.h"

#include "esp_zigbee_cluster.h"
#include "zcl/esp_zigbee_zcl_thermostat.h"
#include "zb_constants_helper.h"
#include "logging.h"
#include "zb_debug.h"
#include "zb_helper.h"

ZigbeeStelproH420Thermostat::ZigbeeStelproH420Thermostat(uint8_t endpoint) : ZigbeeEP(endpoint) {
  /*
  // HACK for debugging logEntry() calls since ZigbeeStelproH420Thermostat as a static object before setup() is called.
  // Wait up to 3s for serial
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  */

  _device_id = ESP_ZB_HA_THERMOSTAT_DEVICE_ID;
  
  // Initialize callbacks to nullptr
  _on_temperature_change = nullptr;
  _on_occupied_cool_setpoint_change = nullptr;
  _on_occupied_heat_setpoint_change = nullptr;
  _on_outdoor_temperature_change = nullptr;
  _on_system_mode_change = nullptr;
  _on_pi_heating_demand_change = nullptr;
  _on_running_state_change = nullptr;
  _on_display_mode_change = nullptr;
  _on_keypad_lockout_change = nullptr;

  // Initialize default values
  _outdoor_temperature = 0;
#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  _stelpro_outdoor_temp = 0;
#endif // #ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
  _pi_heating_demand = 0;
  _running_state = THERMOSTAT_RUNNING_STATE_IDLE;
  _temperature_display_mode = ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE; // celcius ? fahrenheit ?
  _keypad_lockout = ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_DEFAULT_VALUE;  // Unlocked
  _occupancy = ESP_ZB_ZCL_THERMOSTAT_OCCUPANCY_DEFAULT_VALUE; // Occupied
  _display_mode = ESP_ZB_ZCL_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_DEFAULT_VALUE;

  // Create cluster configuration
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

void logInvalidAttrDataType(uint16_t cluster_id, esp_zb_zcl_attr_type_t actual_type_id, uint16_t expected_type_id, uint16_t attr_id) {
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;
  esp_zb_zcl_attr_type_t actual_type = (esp_zb_zcl_attr_type_t)actual_type_id;
  esp_zb_zcl_attr_type_t expected_type = (esp_zb_zcl_attr_type_t)expected_type_id;

  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* actual_name = zb_constants_zcl_attr_type_to_string(actual_type);
  const char* expected_name = zb_constants_zcl_attr_type_to_string(expected_type);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  logEntry( "Unexpected data type received: cluster=%s, attr=%s, expected=%s, actual=%s", cluster_name, attr_name, expected_name, actual_name);
}

void logUnknownAttrDataType(uint16_t cluster_id, uint16_t attr_id) {
  esp_zb_zcl_cluster_id_t cluster = (esp_zb_zcl_cluster_id_t)cluster_id;

  const char* cluster_name = zb_constants_cluster_id_to_string(cluster);
  const char* attr_name = zb_constants_smart_cluster_attr_to_string(cluster, attr_id);

  logEntry("Received unknown message: cluster=%s (0x%04x), attr=%s (0x%04x)", cluster_name, cluster_id, attr_name, attr_id );
}

void ZigbeeStelproH420Thermostat::zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {
  // Handle incoming Zigbee attribute changes.
  // When this function is called, the internal memory of the attribute is already updated.
  // There is no need to call setters.
  
  if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT) {
    // Thermostat cluster
    switch (message->attribute.id) {
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logInvalidAttrDataType(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
          return;
        }
        int16_t new_setpoint = *(int16_t *)message->attribute.data.value;
        int16_t old_setpoint = 0;
        bool success = getOccupiedHeatingSetpoint(old_setpoint);
        if (old_setpoint != new_setpoint) {
          if (_on_occupied_heat_setpoint_change) {
            _on_occupied_heat_setpoint_change(new_setpoint);
          }
        }
        break;
      }

      case ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
          // ERROR
          logInvalidAttrDataType(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
          return;
        }
        uint8_t new_mode = *(uint8_t *)message->attribute.data.value;
        uint8_t old_mode = 0;
        bool success = getSystemMode(old_mode);
        if (old_mode != new_mode) {
          if (_on_system_mode_change) {
            _on_system_mode_change(new_mode);
          }
        }
        break;
      }

#ifdef ENABLE_STELPRO_CUSTOM_ATTR_OUTDOOR_TEMP
      case ZB_ZCL_ATTR_THERMOSTAT_STELPRO_OUTDOOR_TEMP_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_S16) {
          // ERROR
          logInvalidAttrDataType(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_S16, message->attribute.id);
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
        logUnknownAttrDataType(message->info.cluster, message->attribute.id);
        break;
    }
  } 
  else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG) {
    // Thermostat UI Config cluster
    switch (message->attribute.id) {
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_U8) {
          // ERROR
          logInvalidAttrDataType(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_U8, message->attribute.id);
          return;
        }
        uint8_t new_display_mode = *(uint8_t *)message->attribute.data.value;
        if (_display_mode != new_display_mode) {
          _display_mode = new_display_mode;
          if (_on_display_mode_change) {
            _on_display_mode_change(_display_mode);
          }
        }
        break;
      }
      case ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT_ID:
      {
        if (message->attribute.data.type != ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM) {
          // ERROR
          logInvalidAttrDataType(message->info.cluster, message->attribute.data.type, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, message->attribute.id);
          return;
        }
        uint8_t new_keypad_lockout = *(uint8_t *)message->attribute.data.value;
        if (_keypad_lockout != new_keypad_lockout) {
          _keypad_lockout = new_keypad_lockout;
          if (_on_keypad_lockout_change) {
            _on_keypad_lockout_change(_keypad_lockout);
          }
        }
        break;
      }
      default:
        logUnknownAttrDataType(message->info.cluster, message->attribute.id);
        break;
    };
  }
  else {
    // ERROR
    logUnknownAttrDataType(message->info.cluster, message->attribute.id);
  }
}

// Setter methods with Zigbee attribute updates
bool ZigbeeStelproH420Thermostat::setLocalTemperature(int16_t temperature) {
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID, temperature);
  if (!success)
    return false;

  if (_on_temperature_change) {
    _on_temperature_change(temperature);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedCoolingSetpoint(int16_t setpoint) {
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_ID, setpoint);
  if (!success)
    return false;

  if (_on_occupied_cool_setpoint_change) {
    _on_occupied_cool_setpoint_change(setpoint);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setOccupiedHeatingSetpoint(int16_t setpoint) {
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID, setpoint);
  if (!success)
    return false;

  if (_on_occupied_heat_setpoint_change) {
    _on_occupied_heat_setpoint_change(setpoint);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setSystemMode(uint8_t mode) {
  bool success = setGenericAttribute(ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID, mode);
  if (!success)
    return false;

  if (_on_system_mode_change) {
    _on_system_mode_change(mode);
  }
  return success;
}

bool ZigbeeStelproH420Thermostat::setPIHeatingDemand(uint8_t demand) {
  esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

  // logEntry("Updating pi heating demand to %d", demand);

  // Update cluster
  esp_zb_lock_acquire(portMAX_DELAY);
  // set attribute value
  ret = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID,
    &_pi_heating_demand,
    false
  );
  if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set pi heating demand. Status: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
    goto unlock_and_return;
  }
  
  // Update attribute
  _pi_heating_demand = demand;
  if (_on_pi_heating_demand_change) {
    _on_pi_heating_demand_change(_pi_heating_demand);
  }
unlock_and_return:
  esp_zb_lock_release();
  return ret == ESP_ZB_ZCL_STATUS_SUCCESS;
}

bool ZigbeeStelproH420Thermostat::setRunningState(uint16_t state) {
  esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

  // logEntry("Updating running state to %d", state);

  // Update cluster
  esp_zb_lock_acquire(portMAX_DELAY);
  // set attribute value
  ret = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID,
    &_running_state,
    false
  );
  if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set running state. Status: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
    goto unlock_and_return;
  }
  
  // Update attribute
  _running_state = state;
  if (_on_running_state_change) {
    _on_running_state_change(_running_state);
  }
unlock_and_return:
  esp_zb_lock_release();
  return ret == ESP_ZB_ZCL_STATUS_SUCCESS;
}

bool ZigbeeStelproH420Thermostat::setOutdoorTemperature(int16_t temperature) {
  esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

  // logEntry("Updating outdoor temperature to %d", temperature);

  // Update cluster
  esp_zb_lock_acquire(portMAX_DELAY);
  // set attribute value
  ret = esp_zb_zcl_set_attribute_val(
    _endpoint,
    ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID,
    &_outdoor_temperature,
    false
  );
  if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logEntry("Failed to set outdoor temperature. Status: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
    goto unlock_and_return;
  }
  
  // Update attribute
  _outdoor_temperature = temperature;
  if (_on_outdoor_temperature_change) {
    _on_outdoor_temperature_change(temperature);
  }
unlock_and_return:
  esp_zb_lock_release();
  return ret == ESP_ZB_ZCL_STATUS_SUCCESS;
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

/**
 * @brief Print the current registered list of clusters and their registered attributes.
 */
void ZigbeeStelproH420Thermostat::debugClusterList() {
  debugPrintClusterList(_cluster_list);
}

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
    logEntry("Unexpected attribute size for attribute 0x%04x (%s) of cluster 0x%04x (%s). Output size:%d Attribute size:%d", attr_id, attr_name, cluster_id, cluster_name, output_size, attr_size);
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
    logEntry("Unexpected attribute size from cluster 0x%04x (%s) attribute 0x%04x (%s). Output size:%d Attribute size:%d", cluster_id, cluster_name, attr_id, attr_name, value_size, attr_size);
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

esp_zb_cluster_list_t * ZigbeeStelproH420Thermostat::zigbee_stelpro_thermostat_clusters_create(zigbee_stelpro_thermostat_cfg_t *thermostat_cfg) {
  esp_err_t err = ESP_OK;

  // Create clusters from device config with mandatory attributes
  esp_zb_attribute_list_t *esp_zb_basic_cluster_attribute_list = esp_zb_basic_cluster_create(&thermostat_cfg->basic_cfg);
  esp_zb_attribute_list_t *esp_zb_identify_cluster_attribute_list = esp_zb_identify_cluster_create(&thermostat_cfg->identify_cfg);
  esp_zb_attribute_list_t *esp_zb_groups_cluster_attribute_list = esp_zb_groups_cluster_create(&thermostat_cfg->groups_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_cluster_attribute_list = esp_zb_thermostat_cluster_create(&thermostat_cfg->thermostat_cfg);
  esp_zb_attribute_list_t *esp_zb_thermostat_ui_config_cluster_attribute_list = esp_zb_thermostat_ui_config_cluster_create(&thermostat_cfg->thermostat_ui_config_cfg);

  // Add additional thermostat attributes
  //err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_PI_HEATING_DEMAND_ID, &_pi_heating_demand);       logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID, &_running_state);    logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID, &_outdoor_temperature);   logError(err, __FILE__, __LINE__);
  err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPANCY_ID, &_occupancy);                       logError(err, __FILE__, __LINE__);

  // Force HEATING MIN/MAX SETPOINTS
  #if 1
  {
    // Heating setpoint limits (even for heating-only thermostats)
    static int16_t abs_min_heat_setpoint =  STELPRO_MIN_HEAT_SETPOINT;
    static int16_t abs_max_heat_setpoint =  STELPRO_MAX_HEAT_SETPOINT;
    static int16_t min_heat_setpoint =      STELPRO_MIN_HEAT_SETPOINT;
    static int16_t max_heat_setpoint =      STELPRO_MAX_HEAT_SETPOINT;

    // Add attribute limits
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_ID, &abs_min_heat_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_ID, &abs_max_heat_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID,     &min_heat_setpoint);       ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID,     &max_heat_setpoint);       ZB_LOG_ERROR(err);
  }
  #endif

  // Force COOLING MIN/MAX SETPOINTS
  #if 1
  {
    // Cooling setpoint limits (even for heating-only thermostats)
    static int16_t abs_min_cool_setpoint =  STELPRO_MIN_COOL_SETPOINT;
    static int16_t abs_max_cool_setpoint =  STELPRO_MAX_COOL_SETPOINT;
    static int16_t min_cool_setpoint =      STELPRO_MIN_COOL_SETPOINT;
    static int16_t max_cool_setpoint =      STELPRO_MAX_COOL_SETPOINT;

    // Add attribute limits
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_ID, &abs_min_cool_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_ID, &abs_max_cool_setpoint);   ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_ID,     &min_cool_setpoint);       ZB_LOG_ERROR(err);
    err = esp_zb_thermostat_cluster_add_attr(esp_zb_thermostat_cluster_attribute_list, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_ID,     &max_cool_setpoint);       ZB_LOG_ERROR(err);
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
  esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list,                esp_zb_basic_cluster_attribute_list,                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list,             esp_zb_identify_cluster_attribute_list,              ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_groups_cluster(esp_zb_cluster_list,               esp_zb_groups_cluster_attribute_list,                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_cluster(esp_zb_cluster_list,           esp_zb_thermostat_cluster_attribute_list,            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_thermostat_ui_config_cluster(esp_zb_cluster_list, esp_zb_thermostat_ui_config_cluster_attribute_list,  ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  return esp_zb_cluster_list;
}

