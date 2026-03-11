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

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "ZigbeeStelproH420Thermostat.h"
#include "RgbLedBlinker.h"
#include "Button2.h"
#include <SoftTimers.h>
#include "logging.h"
#include "scope_debugger.h"
#include "zb_helper.h"
#include "ZigbeeAttributeT.hpp"

#ifdef ENABLE_DAIKINHTTP
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "DaikinHTTP.h"
#endif // ENABLE_DAIKINHTTP

// Pin definitions
#define LED_PIN RGB_BUILTIN   // RGB LED on ESP32-C6
#define BUTTON_PIN BOOT_PIN   // BOOT button on ESP32-C6

// Temperature simulation timing
#define SIMULATION_UPDATE_INTERVAL             5000 //  5.0 seconds
#define SIMULATION_DEFAULT_ROOM_TEMPERATURE   2000  // 20.0°C
#define SIMULATION_DEFAULT_HEATING_SETPOINT   2400  // 24.0°C
#define SIMULATION_TEMPERATURE_DIFF_HIGH       500  //  5.0°C
#define SIMULATION_TEMPERATURE_STEP_LOW       (1 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_STEP_LOW       (1 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_STEP_HIGH      (5 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_THRESHOLD      50  // 0.5°C

// Factory reset delay
#define FACTORY_RESET_LONG_CLICK_TIME 3 // in seconds, to press and hold button for factory reset

// RGB LED blinker
RgbLedBlinker blinker;
enum LED_MODE {
  LED_MODE_OFF,
  LED_MODE_IDENTIFY,
  LED_MODE_DISCONNECTED,
  LED_MODE_CONNECTED,
  LED_MODE_PEAK_DEMAND_EVENT,
};
LED_MODE previousLedMode = LED_MODE_OFF;

// ZigbeeStelproH420Thermostat invokes Zigbee API functions inside its constructor.
// Therefore, the object must be created dynamically (on the heap) during setup().
// This avoids calling Zigbee functions during static initialization, which occurs before setup() runs.
// During static initialization, a FreeRTOS task executes in parallel, causing a race condition that crashes the ESP32‑C6.
ZigbeeStelproH420Thermostat* zbThermostat = nullptr;

// Button handler
Button2 button;

// Update timers
SoftTimer tempSimulationUpdateTimer;
SoftTimer identifyTimer;

bool peak_demand = false;

// -------------------------------------------------------------------------
//                          Daikin support section
// -------------------------------------------------------------------------
#ifdef ENABLE_DAIKINHTTP
DaikinHTTP daikin(SECRET_DAIKIN_HEATPUMP_IP);

void daikinIncreaseTargetTempBy1() {
  // Pull to refresh latest data
  Serial.println("Pulling device info...");
  if (!daikin.pull()) {
    Serial.println("Failed to pull device info.");
    return;
  }
  Serial.println("pulled!");
  Serial.println("actual    payload=" + daikin.getControlInfoPayload().get());

  float target_temp = daikin.getTargetTemp();
  target_temp += 1.0;
  daikin.setTargetTemp(target_temp);
  Serial.println("temporary payload=" + daikin.getControlInfoPayload().get());

  Serial.println("Pushing new device info...");
  if (!daikin.push()) {
    Serial.println("Failed to push new device info.");
    return;
  }
  Serial.println("pushed!");

  // Pull again to refresh changes
  Serial.println("Pulling device info...");
  if (!daikin.pull()) {
    Serial.println("Failed to pull device info.");
    return;
  }
  Serial.println("pulled!");
  Serial.println("actual    payload=" + daikin.getControlInfoPayload().get());
}

void daikinPullAndPrintInfo() {
  if (!daikin.pull()) {
    Serial.println("Failed to pull device info.");
    return;
  }

  // Print payloads
  Serial.print("DEBUG: Basic payload: "); Serial.println(daikin.getBasicInfoPayload().get());
  Serial.print("DEBUG: Control payload: "); Serial.println(daikin.getControlInfoPayload().get());
  Serial.print("DEBUG: Sensor payload: "); Serial.println(daikin.getSensorInfoPayload().get());
  Serial.println();

  // Print basic info
  {
    String device_name = daikin.getDeviceName();
    Serial.print("Device name:  "); Serial.println(device_name);
  }
  
  // Print control info
  {
    DaikinHTTP::Power power = daikin.getPower();
    DaikinHTTP::Mode mode = daikin.getMode();
    DaikinHTTP::FanRate fan = daikin.getFanRate();
    DaikinHTTP::FanDir FanDir = daikin.getFanDir();
    DaikinHTTP::Preset preset = daikin.getPreset();
    float target_temp = daikin.getTargetTemp();

    Serial.println("Power:        " + DaikinHTTP::toString(power));
    Serial.println("Mode:         " + DaikinHTTP::toString(mode));
    Serial.println("Fan rate:     " + DaikinHTTP::toString(fan));
    Serial.println("Fan dir:      " + DaikinHTTP::toString(FanDir));
    Serial.println("Preset:       " + DaikinHTTP::toString(preset));
    Serial.println("Target Temp:  " + String(target_temp));
  }

  // Print sensor info
  {
    float indoor_temp = daikin.getIndoorTemp();
    float outdoor_temp = daikin.getOutdoorTemp();

    Serial.println("Indoor Temp:  " + String(indoor_temp));
    Serial.println("Outdoor Temp: " + String(outdoor_temp));
  }
}

void daikinSetup() {
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Increase current target temperature by 1 degree
  daikinIncreaseTargetTempBy1();
}
#endif // ENABLE_DAIKINHTTP

// -------------------------------------------------------------------------
//                          Reset/init functions
// -------------------------------------------------------------------------
void initTempSimulationUpdateTimer() {
  tempSimulationUpdateTimer.setTimeOutTime(SIMULATION_UPDATE_INTERVAL);
  tempSimulationUpdateTimer.reset();
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

  Serial.println("Factory reset triggered - hold detected for " + String(FACTORY_RESET_LONG_CLICK_TIME) + " seconds!");
  Serial.println("Rebooting in 1 second...");
  delay(1000);
  Zigbee.factoryReset();
}

void longClickDetected(Button2& btn) {
  //Serial.print("button long click detected!");
}

// -------------------------------------------------------------------------
//                          Temperature Simulation
// -------------------------------------------------------------------------

void printAllAttributes() {
  
  log_i("attributes: {");

  // Get and show all thermostat attributes
  ZigbeeStelproH420Thermostat::zb_zcl_stelpro_thermostat_snapshot_t actuals = {0};
  bool readed = zbThermostat->getSnapshot(actuals);
  if (!readed)
    log_i("ERROR: Failed to read snapshot!");
  zbThermostat->printSnapshot(actuals);

  log_i("};");
}

void simulateTemperature() {
  if (!tempSimulationUpdateTimer.hasTimedOut()) {
    return;
  }
  
  // reset timer for next iteration
  initTempSimulationUpdateTimer();

  // Get current values
  int16_t local_temp = 0;
  int16_t setpoint = 0;
  uint8_t pi_heating_demand = 0;
  uint16_t running_state = 0;
  uint8_t system_mode = 0;
  uint16_t power = 0;
  uint32_t energy = 0;

  // Get actuals
  zbThermostat->getLocalTemperature(local_temp);
  zbThermostat->getOccupiedHeatingSetpoint(setpoint);
  zbThermostat->getPIHeatingDemand(pi_heating_demand);
  zbThermostat->getRunningState(running_state);
  zbThermostat->getSystemMode(system_mode);
  zbThermostat->getStelproPower(power);
  zbThermostat->getStelproEnergy(energy);

  int16_t new_local_temp = local_temp;
  uint8_t new_pi_heating_demand = pi_heating_demand;
  uint16_t new_running_state = running_state;
  uint16_t new_power = power;
  uint32_t new_energy = energy;

  // Compute energy use since las update and at it to the previous energy value
  static const double MILLISECONDS_TO_SECONDS = 0.001;
  static const double SECONDS_TO_HOURS = 1/3600.0;
  double elapsed_hours_since_last_update = tempSimulationUpdateTimer.getTimeOutTime()  * MILLISECONDS_TO_SECONDS * SECONDS_TO_HOURS;
  uint32_t energy_use_deltaT = (uint32_t)(power * elapsed_hours_since_last_update);
  new_energy = energy + energy_use_deltaT;

  int16_t temp_diff = setpoint - local_temp; // positive when requiring heating
  if (abs(temp_diff) < SIMULATION_TEMPERATURE_THRESHOLD) {
    // Nothing to do.
    // Set everything to OFF, temperature do not need to change.
    new_pi_heating_demand = 0;
    new_running_state = THERMOSTAT_RUNNING_STATE_IDLE;
    new_power = 0;
  } else {
    // Temperature difference is above the threshold. Temperature must change and system must be updated.

    // Compute new system mode based on current situation setpoint vs local_temp.
    if (setpoint > local_temp) {
      // Thermostat must HEAT
      new_running_state |= ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT;
    } else {
      new_running_state = THERMOSTAT_RUNNING_STATE_IDLE;
      new_power = 0;
    }

    int16_t target_temp = setpoint;
    
    // Compute target temperature
    if (new_running_state == THERMOSTAT_RUNNING_STATE_IDLE) {
      // When off, temperature drifts toward room temp
      target_temp = SIMULATION_DEFAULT_ROOM_TEMPERATURE; // SIMULATION_DEFAULT_ROOM_TEMPERATURE is the new setpoint
    }

    // Update new_temp towards target temperature
    if (local_temp < target_temp - SIMULATION_TEMPERATURE_DIFF_HIGH ) {
      new_local_temp += SIMULATION_TEMPERATURE_STEP_HIGH;   // Heat faster when far from setpoint
    } else if (local_temp < target_temp) {
      new_local_temp += SIMULATION_TEMPERATURE_STEP_LOW;    // Heat slower when close to setpoint
    } else if (local_temp > target_temp + SIMULATION_TEMPERATURE_DIFF_HIGH) {
      new_local_temp -= SIMULATION_TEMPERATURE_STEP_HIGH;   // Cool faster when too hot
    } else if (local_temp > target_temp) {
      new_local_temp -= SIMULATION_TEMPERATURE_STEP_LOW;    // Cool slower when close
    }

    // Compute new pi_heating_demand
    if ((new_running_state | ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT) > 0) {
      int16_t tmp = (int16_t)map(abs(temp_diff), 0000, 0300, (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE, (int16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE); // map [0°C,3°C] to [0%,100%]
      if (tmp > 100) tmp = 100;
      if (tmp < 0) tmp = 0;
      new_pi_heating_demand = (uint8_t)tmp;

      // compute new power
      new_power = (uint16_t)map(new_pi_heating_demand, (uint16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE, (uint16_t)ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE, 0, STELPRO_MAX_POWER_WATTS); // map [0%,100%] to [0W,4000W]
    }
  }

  // Update Energy
  if (pi_heating_demand > 0) {
    zbThermostat->setStelproEnergy(new_energy);
  }

  // Note:
  // Zigbee attribute `pi_heating_demand` can not be set to a value (even 0) if :
  // * attribute `system_mode` is IDLE or
  // * HEATING bit in `running_state` is not set.
  // So we must set this flag sometimes before (and sometimes after) `system_mode` and `running_state` to make sure it is accepted.
  bool pi_heating_demand_is_dirty = (new_pi_heating_demand != pi_heating_demand);
  uint16_t tmp_running_state = 0;
  if (pi_heating_demand_is_dirty && zbThermostat->getRunningState(tmp_running_state) && (tmp_running_state & ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT)) {
    // If heating bit is already set, it is safe to update pi_heating_demand now.
    // This cover use case when `running_state` transitions from `HEATING` --> `IDLE`.
    // We must update `pi_heating_demand` before HEATING flag in `runnig_state` is cleared.
    zbThermostat->setPIHeatingDemand(new_pi_heating_demand);
    pi_heating_demand_is_dirty = false;

    // Update current power usage
    zbThermostat->setStelproPower(new_power);
  }

  // Update thermostat attribute, if changed
  if (new_running_state != running_state) {
    zbThermostat->setRunningState(new_running_state);
  }
  if (new_local_temp != local_temp) {
    zbThermostat->setLocalTemperature(new_local_temp);
  }
  
  // If pi_heating_demand is still dirty, we must update it now
  if (pi_heating_demand_is_dirty) {
    // If pi_heating_demand is still dirty, we update it.
    // This cover use case when `running_state` has just transitioned from `IDLE` --> `HEATING`.
    // We must update `pi_heating_demand` after HEATING flag in `runnig_state` is set.
    zbThermostat->setPIHeatingDemand(new_pi_heating_demand);
    pi_heating_demand_is_dirty = false;

    // Update current power usage
    zbThermostat->setStelproPower(new_power);
  }
  
  log_i("Simulation Update --> Temp: %.1f°C, Setpoint: %.1f°C, Demand: %d%%, State: %s, Power: %dW, Energy: %dWh",
                new_local_temp / 100.0,
                setpoint / 100.0,
                new_pi_heating_demand,
                zb_constants_zcl_thermostat_running_state_attr_to_string(new_running_state).c_str(),
                new_power,
                new_energy);

  //
  printAllAttributes();

  // force reportable attributes to report their values to the controller
  if (!zbThermostat->report()) {
    log_e("Failed to report zbThermostat!");
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
}

void onOccupiedHeatSetpointChange(int16_t setpoint) {
  log_i("Occupied Heat Setpoint changed from coordinator to: %.1f°C", setpoint / 100.0);
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
  if (identifyTimer.getTimeOutTime() != 0 && !identifyTimer.hasTimedOut()) { // if active and not timed out
    msg = "Indentify - LED set to fast YELLOW blink";
    newLedMode = LED_MODE_IDENTIFY;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_YELLOW);
  } else if (peak_demand) {
    msg = "PEAK DEMAND EVENT - LED set to slow PURPLE pulse";
    newLedMode = LED_MODE_PEAK_DEMAND_EVENT;
    blinker.set(RgbLedBlinker::MODE_PULSE_ONCE_PER_15_SECONDS, RgbLedBlinker::COLOR_PURPLE);
  } else if (!Zigbee.connected()) {
    msg = "Disconnected - LED set to fast RED blink";
    newLedMode = LED_MODE_DISCONNECTED;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_RED);
  } else {
    msg = "Connected - LED set to slow BLUE pulse";
    newLedMode = LED_MODE_CONNECTED;
    blinker.set(RgbLedBlinker::MODE_PULSE_ONCE_PER_15_SECONDS, RgbLedBlinker::COLOR_BLUE);
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

  // Initialize RGB LED blinker
  blinker.setup(LED_PIN);
  blinker.set(RgbLedBlinker::MODE_OFF, RgbLedBlinker::COLOR_BLACK);
  blinker.loop(); // force the LED to turn off

  // Wait up to 3s for serial
  while (!Serial && millis() < 3000);
  
#ifdef ENABLE_DAIKINHTTP
  daikinSetup();
#endif // ENABLE_DAIKINHTTP
  
  log_i("========================================");
  log_i("  Stelpro HT402 Thermostat Emulator");
  log_i("========================================");
  log_i("Model: HT402 (Hilo)");
  log_i("Manufacturer: Stelpro");
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
  
  // Initialize temperature update timer
  initTempSimulationUpdateTimer();
  
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

  //log_i("Waiting 5000 ms...");
  //delay(5000);

  // Init zbThermostat's zigbee attributes
  if (!zbThermostat->setup()) {
    log_i("WARNING: zbThermostat->setup() has failed!");
  }
  
  // Initialize simulation stuff
  zbThermostat->setLocalTemperature(SIMULATION_DEFAULT_ROOM_TEMPERATURE);
  zbThermostat->setOccupiedHeatingSetpoint(SIMULATION_DEFAULT_HEATING_SETPOINT);
  
  // Init identifyTimer
  identifyTimer.setTimeOutTime(0);
  identifyTimer.reset();

  log_i("Connecting to network...");
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

  log_i("Connected to network!");

  // Connected - switch to blue pulse
  updateLEDStatus();
  blinker.loop();

  // DEBUG
  printAllAttributes();
}

void loop() {
  // Update peak_demand flag before updateLEDStatus() since it affect the LED
  peak_demand = false;
  uint16_t peak_demand_remaining_time = 0;
  if (zbThermostat->getStelproPeakDemandIcon(peak_demand_remaining_time)) {
    peak_demand = (peak_demand_remaining_time > 0);
  }

  // Update LED status based on connection state
  updateLEDStatus();
  
  // Disable identifyTimer, after timed out
  if (identifyTimer.getTimeOutTime() != 0 && identifyTimer.hasTimedOut()) { // if active and has timed out
    identifyTimer.setTimeOutTime(0);
    identifyTimer.reset();
  }

  // Update all components
  blinker.loop();
  button.loop();
  if (!zbThermostat->update()) { // remember previous values
    log_w("zbThermostat->update() has failed");
  }
  zbThermostat->updateEnergy();  // Update energy simulation calculations and Zigbee attributes
  simulateTemperature();

  delay(10);
}
