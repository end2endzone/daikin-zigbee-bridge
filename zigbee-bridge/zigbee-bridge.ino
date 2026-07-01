/**
 * @brief Zigbee Stelpro H420/HT402 Thermostat Emulator
 *
 * This sketch emulates a Stelpro HT402 (Hilo) line-voltage thermostat
 * for Zigbee2MQTT recognition. The HT402 is a heating-only thermostat
 * designed for baseboard heaters (4000W @ 240V).
 *
 * Hardware:
 * - ESP32-C6 with onboard LED on GPIO8 (RGB LED)
 * - RGB LED on GPIO8
 * - BOOT button on GPIO9
 *
 * LED Indicators:
 * - Fast red blink: Pairing mode (unpaired) or disconnected.
 * - Fast yellow blink: Identify mode active
 * - Slow blue pulse (once per 15s): Connected to Zigbee coordinator
 *
 * Button:
 * - Press and hold for 3 seconds: Factory reset and enter pairing mode
 *
 * Arduino IDE Settings:
 * - Board: ESP32C6 Dev Module
 * - Zigbee Mode: Zigbee ED (end device)
 * - Partition Scheme: Zigbee 4MB with spiffs
 *
 * Required Libraries:
 * - Button2 (install via Library Manager)
 * - SoftTimers (install via Library Manager)
 */

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

//#define ENABLE_DAIKIN_SERIAL_MOCK

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "ZigbeeStelproH420Thermostat.h"
#include "RgbLedBlinker.h"
#include "Button2.h"
#include <SoftTimers.h>
#include "logging.h"
#include "scope_debugger.h"
#include "zb_helper.h"
#include "project_config.h"
#include "ZigbeeAttributeT.hpp"
#ifdef ENABLE_DAIKIN_SERIAL_MOCK
#include "DaikinSerialLocalMock.h"
#else
#include "DaikinSerialClient.h"
#endif

#define UART0_TX 17
#define UART0_RX 16
#define UART1_TX 18
#define UART1_RX 19

// Pin definitions
#define LED_PIN RGB_BUILTIN   // RGB LED on ESP32-C6
#define BUTTON_PIN BOOT_PIN   // BOOT button on ESP32-C6

// Temperature synchronisation timing
#define TEMPERATURE_SYNC_UPDATE_INTERVAL  30000  // 30.0 seconds

// Factory reset delay
#define FACTORY_RESET_LONG_CLICK_TIME 3 // in seconds, to press and hold button for factory reset

#define FORCE_REPORTING_INTERVAL           30000  // 30.0 seconds

// RGB LED blinker
RgbLedBlinker blinker;
enum LED_MODE {
  LED_MODE_OFF,
  LED_MODE_IDENTIFY,
  LED_MODE_ZIGBEE_DISCONNECTED,
  LED_MODE_WIFI_DISCONNECTED,
  LED_MODE_DAIKIN_OFFLINE,
  LED_MODE_CONNECTED,
  LED_MODE_PEAK_DEMAND_EVENT,
};
LED_MODE previousLedMode = LED_MODE_OFF;

// ZigbeeStelproH420Thermostat invokes Zigbee API functions inside its constructor.
// Therefore, the object must be created dynamically (on the heap) during setup().
// This avoids calling Zigbee functions during static initialization, which occurs before setup() runs.
// During static initialization, a FreeRTOS task executes in parallel, causing a race condition that crashes the ESP32‑C6.
ZigbeeStelproH420Thermostat* zbThermostat = nullptr;

#ifdef ENABLE_DAIKIN_SERIAL_MOCK
DaikinSerialLocalMock daikin;
#else
DaikinSerialClient daikin;
#endif

// Button handler
Button2 button;

// Update timers
SoftTimer syncUpdateTimer;
SoftTimer forceReportingTimer;
SoftTimer identifyTimer;

bool peak_demand = false;
bool forceSync = false; // a flag that can be set to force the synchronization between DaikinSerialApi and the Zigbee Thermostat.

// -------------------------------------------------------------------------
//                          Reset/init functions
// -------------------------------------------------------------------------
void initSyncUpdateTimer() {
  syncUpdateTimer.setTimeOutTime(TEMPERATURE_SYNC_UPDATE_INTERVAL);
  syncUpdateTimer.reset();
}

void initForceReportingTimer() {
  forceReportingTimer.setTimeOutTime(FORCE_REPORTING_INTERVAL);
  forceReportingTimer.reset();
}

// -------------------------------------------------------------------------
//                            Button Callbacks
// -------------------------------------------------------------------------

void clickDetected(Button2& btn) {
  //Serial.print("button click detected!");
}

void doubleClickDetected(Button2& btn) {
  //Serial.print("button double click detected!");
}

void trippleClickDetected(Button2& btn) {
  //Serial.print("button tripple click detected!");
}

void holdDetected(Button2& btn) {
  //Serial.println("button hold detected!");

  log_i("Factory reset triggered - hold detected for %d seconds!", FACTORY_RESET_LONG_CLICK_TIME);
  log_i("Rebooting in 1 second...");
  delay(1000);
  Zigbee.factoryReset();
}

void longClickDetected(Button2& btn) {
  //Serial.print("button long click detected!");
}

// -------------------------------------------------------------------------
//                  Temperature Simulation & synchronization        
// -------------------------------------------------------------------------

/**
 * @brief Force a Daikin Serial to Zigbee Thermostat synchronization.
 * Daikin controller can change state without the zigbee bridge to be notified.
 * This function make sure to synchronize the zigbee bridge from the Daikin Serial adaptor.
 * 
 * Returns true when the synchronization is succesful. Returns false otherwise.
 */
bool forceDaikinSerialToZigbeeThermostatSynchronization() {
  // Note: Zigbee target temperature updates are synchronized synchronously in the zigbee callback.

  // reset the force sync flag, if set.
  forceSync = false;

  log_i("Synchronizing zigbee controller from daikin serial adapter.");

  // Daikin Serial remote updates must be manually pulled.
  DaikinSerialApi::daikin_status_info_t remote_status = {};
  DaikinSerialApi::ApiResult result = daikin.getStatus(&remote_status);
  if (result != DaikinSerialApi::ApiResult::API_RESULT_OK) {
    log_e("Failed to get Remote Status from Daikin Controller: %s", DaikinSerialApi::toString(result).c_str());
    return false;
  } else {
    // If manual pull was succesful. Push values to the zigbee-bridge.

    // Set setpoint
    if (!zbThermostat->setOccupiedHeatingSetpoint(remote_status.target_temp)) {
      log_e("Unable to set setpoint. Synchronization has failed.");
      return false;
    }

    // Set local temperature
    if (!zbThermostat->setLocalTemperature(remote_status.indoor_temp)) {
      log_e("Unable to set local temperature. Synchronization has failed.");
      return false;
    }

    // Update the heating logic of the thermostat manually.
    // See call to ZigbeeStelproH420Thermostat::setManualHeatingLogicUpdate() in setup() function.
    bool is_heating = DaikinSerialApi::isHeating(&remote_status);
    bool is_cooling = DaikinSerialApi::isCooling(&remote_status);
    uint16_t new_running_state = 0;
    uint8_t new_pi_heating_demand = 0;
    uint16_t new_stelpro_power = 0;
    if (is_heating || is_cooling)
      new_running_state |= ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT;
    if (new_running_state & ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT) {
      // map [0.0,to 1.0] to [0%,100%]
      int16_t tmp = (int16_t)map(
        DaikinSerialApi::getEstimatedInstantaneousPowerRatio(&remote_status),
        0.0,
        1.0,
        (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE,
        (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE);
      if (tmp > 100) tmp = 100;
      if (tmp < 0) tmp = 0;
      new_pi_heating_demand = (uint8_t)tmp;
    }
    new_stelpro_power = DaikinSerialApi::getEstimatedInstantaneousPower(&remote_status, NUM_INDOOR_UNIT);
    if (!zbThermostat->setHeatingLogic(new_running_state, new_pi_heating_demand, new_stelpro_power)) {
      log_e("Unable to set heating logic. Synchronization has failed.");
      return false;
    }

    // Force reporting all changed attributes
    if (!zbThermostat->report()) {
      log_e("zbThermostat has failed to report())!");
    }
  }

  String status_desc = DaikinSerialApi::toString(&remote_status, NUM_INDOOR_UNIT);
  log_i("Daikin heatpump attributes: %s", status_desc.c_str());

  // print zigbee values as well for debuging and validating they match the Daikin controller's state. 
  printAllAttributes();

  return true;
}

/**
 * @brief Check if a Daikin Serial to Zigbee Thermostat synchronization is required.
 */
void checkDaikinSerialToZigbeeThermostatSynchronization() {
  if (!forceSync) {
    // Make sure we do not call this function too often...
    if (syncUpdateTimer.getTimeOutTime() != 0 && !syncUpdateTimer.hasTimedOut()) {
      return; // too soon
    }
  }
  // reset timer for next iteration timestamps
  syncUpdateTimer.reset();

  forceDaikinSerialToZigbeeThermostatSynchronization();
}

void printAllAttributes() {
  
  log_i("attributes: {");

  // Get and show all thermostat attributes
  ZigbeeStelproH420Thermostat::zb_zcl_stelpro_thermostat_snapshot_t actuals = {};
  bool readed = zbThermostat->getSnapshot(actuals);
  if (!readed)
    log_i("ERROR: Failed to read snapshot!");
  zbThermostat->printSnapshot(actuals);

  log_i("};");
}


void reportAttributes() {
  // Make sure we do not call this function too often...
  if (forceReportingTimer.getTimeOutTime() != 0 && !forceReportingTimer.hasTimedOut()) {
    return; // too soon
  }
  // reset timer for next iteration timestamps
  forceReportingTimer.reset();

  // force reportable attributes to report their values to the controller
  if (!zbThermostat->report()) {
    log_e("zbThermostat has failed to report())!");
  }
}

// -------------------------------------------------------------------------
//                            Zigbee Callbacks
// -------------------------------------------------------------------------

void onIdentify(uint16_t time) {
  log_i("Identify time changed to: %d seconds", time);
  if (time > 0) {
    // Enable identifyTimer timer. This will trigger the LED blinking feedback
    identifyTimer.setTimeOutTime(time * 1000);
    identifyTimer.reset();
  }
}

// Thermostat cluster
void onLocalTemperatureChange(int16_t temperature) {
  log_i("Local Temperature changed from coordinator to: %.1f°C", temperature / 100.0);
}

void onOccupiedCoolSetpointChange(int16_t setpoint) {
  log_i("Occupied Cool Setpoint changed from coordinator to: %.1f°C", setpoint / 100.0);
  
  DaikinSerialApi::ApiResult result = daikin.setTargetTemperature(setpoint);
  if (result != DaikinSerialApi::ApiResult::API_RESULT_OK) {
    log_e("Failed to set Occupied Cool Setpoint in Daikin Controller to: %.1f°C: %s", setpoint / 100.0, DaikinSerialApi::toString(result).c_str());
    return;
  }

  // set the flag to force a sync update.
  forceSync = true;
}

void onOccupiedHeatSetpointChange(int16_t setpoint) {
  log_i("Occupied Heat Setpoint changed from coordinator to: %.1f°C", setpoint / 100.0);
  
  DaikinSerialApi::ApiResult result = daikin.setTargetTemperature(setpoint);
  if (result != DaikinSerialApi::ApiResult::API_RESULT_OK) {
    log_e("Failed to set Occupied Cool Setpoint in Daikin Controller to: %.1f°C: %s", setpoint / 100.0, DaikinSerialApi::toString(result).c_str());
    return;
  }

  // set the flag to force a sync update.
  forceSync = true;
}

void onControlSequenceOfOperationChange(uint8_t csop) {
  log_i("Control Sequence Of Operation changed from coordinator to: %d", csop);
}

void onSystemModeChange(uint8_t mode) {
  log_i("System mode changed from coordinator to: 0x%02x (%s)", mode, zb_constants_zcl_thermostat_system_mode_attr_to_string((esp_zb_zcl_thermostat_system_mode_t)mode));
}

// Thermostat UI cluster
void onDisplayModeChange(uint8_t mode) {
  log_i("Display Mode changed from coordinator to: %d", mode);
}

void onKeypadLockoutChange(uint8_t lockout) {
  log_i("Keypad Lockout changed from coordinator to: %d, %s", lockout, zb_zcl_thermostat_ui_config_keypad_lockout_to_string((zb_zcl_thermostat_ui_config_keypad_lockout_t)lockout));
}

// Thermostat cluster, additional attributes
void onRunningStateChange(uint16_t state) {
  String states_str = zb_constants_zcl_thermostat_running_state_attr_to_string(state);
  log_i("Running State changed from coordinator to: 0x%04x (%s)", state, states_str.c_str());
}

void onPIHeatingDemandChange(uint8_t demand) {
  log_i("PI Heating Demand changed from coordinator to: %d", demand);
}

void onOutdoorTemperatureChange(int16_t temperature) {
  log_i("Outdoor Temperature changed from coordinator to: %.1f°C", temperature / 100.0);
}

void onOccupancyChange(zb_uint8_t occupancy) {
  log_i("Occupancy changed from coordinator to: %d", occupancy);
}

void onStelproOutdoorTemperatureChange(int16_t temperature) {
  log_i("Stelpro Outdoor Temperature changed from coordinator to: %.1f°C", temperature / 100.0);
}

void onStelproSystemModeChange(uint8_t mode) {
  log_i("Stelpro system mode changed from coordinator to: 0x%02x (%s)", mode, zb_constants_zcl_thermostat_system_mode_attr_to_string((esp_zb_zcl_thermostat_system_mode_t)mode));
}

void onStelproPeakDemandIconChange(uint16_t seconds) {
  log_i("Stelpro PeakDemandIcon value changed from coordinator to: %d seconds (%d hours)", seconds, (seconds/3600));
}

// -------------------------------------------------------------------------
//                            LED Status Update
// -------------------------------------------------------------------------
void updateLEDStatus() {
  LED_MODE newLedMode = LED_MODE_OFF;
  const char * msg = "";

  // Check device's state to know how the LED should behave
  if (!Zigbee.connected()) {
    msg = "Zigbee Disconnected - LED set to fast RED blink";
    newLedMode = LED_MODE_ZIGBEE_DISCONNECTED;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_RED);
  } else if (identifyTimer.getTimeOutTime() != 0 && !identifyTimer.hasTimedOut()) { // if active and not timed out
    msg = "Indentify - LED set to fast YELLOW blink";
    newLedMode = LED_MODE_IDENTIFY;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_YELLOW);
  } else if (peak_demand) {
    msg = "PEAK DEMAND EVENT - LED set to slow BLUE pulse";
    newLedMode = LED_MODE_PEAK_DEMAND_EVENT;
    blinker.set(RgbLedBlinker::MODE_PULSE_ONCE_PER_15_SECONDS, RgbLedBlinker::COLOR_BLUE);
  } else {
    msg = "Connected - LED set to slow GREEN pulse";
    newLedMode = LED_MODE_CONNECTED;
    blinker.set(RgbLedBlinker::MODE_PULSE_ONCE_PER_15_SECONDS, RgbLedBlinker::COLOR_GREEN);
  }

  // Did we changed LED MODE ?
  if (previousLedMode != newLedMode) {
    log_i("%s", msg);
  }
  previousLedMode = newLedMode;
}

// -------------------------------------------------------------------------
//                          Arduino main section
// -------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  #ifdef ENABLE_DAIKIN_SERIAL_MOCK
  #else
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
  #endif

  // Initialize RGB LED blinker
  blinker.setup(LED_PIN);
  blinker.set(RgbLedBlinker::MODE_OFF, RgbLedBlinker::COLOR_BLACK);
  blinker.loop(); // force the LED to turn off

  // Wait up to 3s for serial
  while (!Serial && millis() < 3000);
  #ifdef ENABLE_DAIKIN_SERIAL_MOCK
  #else
  while (!Serial1 && millis() < 3000);
  #endif
  
  log_i("========================================");
  log_i("  Stelpro HT402 Thermostat Emulator");
  log_i("========================================");
  log_i("Model: %s (Hilo)", STELPRO_MODEL_NAME);
  log_i("Manufacturer: %s", STELPRO_MANUFACTURER_NAME);
  log_i("Endpoint: %d", STELPRO_ENDPOINT);
  log_i("Type: Line-voltage heating thermostat");
  log_i("========================================");
  
  // Initialize button
  button.begin(BUTTON_PIN);
  button.setDebounceTime(25); // default is 50ms which is too high.
  button.setLongClickTime(FACTORY_RESET_LONG_CLICK_TIME * 1000);
  // Button callbacks
  button.setClickHandler(clickDetected);
  button.setDoubleClickHandler(doubleClickDetected);
  button.setTripleClickHandler(trippleClickDetected);
  button.setLongClickHandler(longClickDetected);
  button.setLongClickDetectedHandler(holdDetected);
  
  // Read the state of the button right now to initialize
  button.loop();

  // Check if the button is already pressed and hold during boot to force a factory reset.
  while (button.isPressed()) {
    button.loop();
  }

  // Initialize temperature update timer
  initSyncUpdateTimer();
  
  // Initialize force reporting timer
  initForceReportingTimer();

  // Change the OUI prefix in the IEEE address (EUI-64 address) 
  // to match `Silicon Laboratories` instead of `Espressif Inc`.
  // This function must be called before calling Zigbee.begin().
  const uint8_t target_oui[3] = { 0xBC, 0x33, 0xAC };
  zb_ieee_addr_set_oui(target_oui);

  // Create the thermostat on the heap.
  zbThermostat = new ZigbeeStelproH420Thermostat(STELPRO_ENDPOINT);

  // Set callback functions for Zigbee
  // Thermostat cluster
  zbThermostat->onIdentify(onIdentify);
  zbThermostat->onLocalTemperatureChange(onLocalTemperatureChange);
  zbThermostat->onOccupiedCoolSetpointChange(onOccupiedCoolSetpointChange);
  zbThermostat->onOccupiedHeatSetpointChange(onOccupiedHeatSetpointChange);
  zbThermostat->onControlSequenceOfOperationChange(onControlSequenceOfOperationChange);
  zbThermostat->onSystemModeChange(onSystemModeChange);
  // Thermostat UI cluster
  zbThermostat->onDisplayModeChange(onDisplayModeChange);
  zbThermostat->onKeypadLockoutChange(onKeypadLockoutChange);
  // Thermostat cluster, additional attributes
  zbThermostat->onRunningStateChange(onRunningStateChange);
  zbThermostat->onPIHeatingDemandChange(onPIHeatingDemandChange);
  zbThermostat->onOutdoorTemperatureChange(onOutdoorTemperatureChange);
  zbThermostat->onOccupancyChange(onOccupancyChange);
  zbThermostat->onStelproOutdoorTemperatureChange(onStelproOutdoorTemperatureChange);
  zbThermostat->onStelproSystemModeChange(onStelproSystemModeChange);
  zbThermostat->onStelproPeakDemandIconChange(onStelproPeakDemandIconChange);

  // Set manufacturer and model
  zbThermostat->setManufacturerAndModel(STELPRO_MANUFACTURER_NAME, STELPRO_MODEL_NAME);

  // The the zigbee thermostat that we will be updating the heating logic manually in forceDaikinSerialToZigbeeThermostatSynchronization().
  zbThermostat->setManualHeatingLogicUpdate(true);
  
  #ifdef ENABLE_DAIKIN_SERIAL_MOCK
  daikin.begin(Serial);
  #else
  daikin.begin(Serial1);
  #endif

  // DEBUG
  //zbThermostat->debugClusterList();
  //log_i("DEBUG: Infinite loop from this point!");
  //while(true) {}
  
  // Add endpoint to Zigbee Core
  log_i("Adding Zigbee Thermostat endpoint to Zigbee Core");
  Zigbee.addEndpoint(zbThermostat);
  
  //log_i("Waiting 5000 ms...");
  //delay(5000);

  log_i("Starting Zigbee stack...");
  if (!Zigbee.begin()) {
    log_i("Zigbee failed to start!");
    log_i("Rebooting...");
    ESP.restart();
  }
  log_i("Zigbee stack ready.");

  // Print the device zigbee ieee address
  esp_zb_ieee_addr_t device_ieee_long_addr = {};
  esp_zb_get_long_address(device_ieee_long_addr); // ZBOSS stores 64-bit of IEEE long address in little-endian order internally, so byte[0] is the LSB.
  log_i("Device zigbee address: %s", zb_ieee_long_addr_to_string(device_ieee_long_addr).c_str());

  //log_i("Waiting 5000 ms...");
  //delay(5000);

  // Init zbThermostat's zigbee attributes
  if (!zbThermostat->setup()) {
    log_i("WARNING: zbThermostat->setup() has failed!");
  }

  // Forcing Zigbee Controller to be initialized with values from the Daikin Serial adaptor.
  while(!forceDaikinSerialToZigbeeThermostatSynchronization()) {
    static const unsigned long INIT_FAILURE_DELAY = 5000;
    log_i("Initializing has failed. Retry again in %u ms.", INIT_FAILURE_DELAY);
    delay(INIT_FAILURE_DELAY);
  }
  
  // Init identifyTimer
  identifyTimer.setTimeOutTime(0);
  identifyTimer.reset();

  log_i("Connecting to zigbee network...");
  size_t dotCount = 0;
  while (!Zigbee.connected()) {
    Serial.print(".");
    dotCount++;
    if (dotCount % 60 == 0)
      Serial.println();

    // Keep LED blinking during connection
    updateLEDStatus();
    blinker.loop();

    delay(100);
  }
  if (dotCount % 60 > 0) // if there is dots printed without a terminating new line
    Serial.println();
  log_i("Connected to zigbee network!");

  // Connected - switch to blue pulse
  updateLEDStatus();
  blinker.loop();

  // Force an update just before printing zigbee attributes
  forceDaikinSerialToZigbeeThermostatSynchronization();

  // DEBUG
  printAllAttributes();
}

void loop() {
  static uint32_t loop_count = 0;
  loop_count++;

  // Update peak_demand flag before updateLEDStatus() since it affect the LED
  peak_demand = false;
  uint16_t peak_demand_remaining_time = 0;
  if (zbThermostat->getStelproPeakDemandIcon(peak_demand_remaining_time)) {
    peak_demand = (peak_demand_remaining_time > 0);
  }
  
  // Disable identifyTimer, after timed out
  if (identifyTimer.getTimeOutTime() != 0 && identifyTimer.hasTimedOut()) { // if active and has timed out
    identifyTimer.setTimeOutTime(0);
    identifyTimer.reset();
  }

  // Update LED status based on connection state
  updateLEDStatus();

  // Update all components
  blinker.loop();
  button.loop();
  if (!zbThermostat->update()) {
    log_w("zbThermostat has failed to update()!");
  }

  #ifdef ENABLE_DAIKIN_SERIAL_MOCK
  daikin.loop();
  #else
  #endif

  // Should we download from daikin and update our zigbee thermostat ?
  checkDaikinSerialToZigbeeThermostatSynchronization();

  reportAttributes();

  delay(10);
}
