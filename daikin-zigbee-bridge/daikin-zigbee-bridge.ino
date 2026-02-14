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
#define SIMULATION_UPDATE_INTERVAL            10000 // 10 seconds
#define SIMULATION_DEFAULT_ROOM_TEMPERATURE   2200  // 22.0°C
#define SIMULATION_TEMPERATURE_DIFF_HIGH       500  //  5.0°C
#define SIMULATION_TEMPERATURE_STEP_LOW       (1 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_STEP_HIGH      (5 * STELPRO_TEMP_MEASUREMENT_TOLERANCE)
#define SIMULATION_TEMPERATURE_SETPOINT       3000  // 30.0°C

// Factory reset delay
#define FACTORY_RESET_LONG_CLICK_TIME 3 // in seconds, to press and hold button for factory reset

// RGB LED blinker
RgbLedBlinker blinker;
enum LED_MODE {
  LED_MODE_OFF,
  LED_MODE_IDENTIFY,
  LED_MODE_DISCONNECTED,
  LED_MODE_CONNECTED,
};
LED_MODE previousLedMode = LED_MODE_OFF;

// Zigbee thermostat (contains EnergyCalculator internally)
ZigbeeStelproH420Thermostat zbThermostat(STELPRO_ENDPOINT);

// Button handler
Button2 button;

// Update timers
SoftTimer tempSimulationUpdateTimer;
SoftTimer identifyTimer;

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
  logEntry("attributes: {");

  // Thermostat cluster
  {
    int16_t actual_local_temperature                = 0;
    int16_t actual_occupied_cooling_setpoint        = 0;
    int16_t actual_occupied_heating_setpoint        = 0;
    uint8_t actual_control_sequence_of_operation    = 0;
    uint8_t actual_system_mode                      = 0;
    if (zbThermostat.getLocalTemperature(           actual_local_temperature              ))    logEntry("  actual_local_temperature=%d",             actual_local_temperature);
    if (zbThermostat.getOccupiedCoolingSetpoint(    actual_occupied_cooling_setpoint      ))    logEntry("  actual_occupied_cooling_setpoint=%d",     actual_occupied_cooling_setpoint);
    if (zbThermostat.getOccupiedHeatingSetpoint(    actual_occupied_heating_setpoint      ))    logEntry("  actual_occupied_heating_setpoint=%d",     actual_occupied_heating_setpoint);
    if (zbThermostat.getControlSequenceOfOperation( actual_control_sequence_of_operation  ))    logEntry("  actual_control_sequence_of_operation=%d", actual_control_sequence_of_operation);
    if (zbThermostat.getSystemMode(                 actual_system_mode                    ))    logEntry("  actual_system_mode=%d",                   actual_system_mode);
  }

  // Thermostat UI cluster
  {
    uint8_t actual_temperature_display_mode     = 0;
    uint8_t actual_keypad_lockout               = 0;
    if (zbThermostat.getTemperatureDisplayMode( actual_temperature_display_mode       ))    logEntry("  actual_temperature_display_mode=%d",            actual_temperature_display_mode);
    if (zbThermostat.getKeypadLockout(          actual_keypad_lockout                 ))    logEntry("  actual_keypad_lockout=%d",                      actual_keypad_lockout);
  }

  // Thermostat cluster, additional attributes
  {
    uint16_t    actual_running_state              = 0;
    uint8_t     actual_pi_heating_demand          = 0;
    int16_t     actual_outdoor_temperature        = 0;
    zb_uint8_t  actual_occupancy                  = 0;
    if (zbThermostat.getRunningState(       actual_running_state                     ))    logEntry("  actual_running_state=%d",              actual_running_state);
    if (zbThermostat.getPIHeatingDemand(    actual_pi_heating_demand                 ))    logEntry("  actual_pi_heating_demand=%d",          actual_pi_heating_demand);
    if (zbThermostat.getOutdoorTemperature( actual_outdoor_temperature               ))    logEntry("  actual_outdoor_temperature=%d",        actual_outdoor_temperature);
    if (zbThermostat.getOccupancy(          actual_occupancy                         ))    logEntry("  actual_occupancy=%d",                  actual_occupancy);

    // Heating setpoints
    {
      int16_t actual_min_heat_setpoint      = 0;
      int16_t actual_max_heat_setpoint      = 0;
      int16_t actual_abs_min_heat_setpoint  = 0;
      int16_t actual_abs_max_heat_setpoint  = 0;
      if (zbThermostat.getMinHeatingSetpointLimit(    actual_min_heat_setpoint    ))    logEntry("  actual_min_heat_setpoint=%d",     actual_min_heat_setpoint);
      if (zbThermostat.getMaxHeatingSetpointLimit(    actual_max_heat_setpoint    ))    logEntry("  actual_max_heat_setpoint=%d",     actual_max_heat_setpoint);
      if (zbThermostat.getAbsMinHeatingSetpointLimit( actual_abs_min_heat_setpoint))    logEntry("  actual_abs_min_heat_setpoint=%d", actual_abs_min_heat_setpoint);
      if (zbThermostat.getAbsMaxHeatingSetpointLimit( actual_abs_max_heat_setpoint))    logEntry("  actual_abs_max_heat_setpoint=%d", actual_abs_max_heat_setpoint);
    }

    // Cooling setpoints
    {
      int16_t actual_min_cool_setpoint      = 0;
      int16_t actual_max_cool_setpoint      = 0;
      int16_t actual_abs_min_cool_setpoint  = 0;
      int16_t actual_abs_max_cool_setpoint  = 0;
      if (zbThermostat.getMinCoolingSetpointLimit(    actual_min_cool_setpoint    ))    logEntry("  actual_min_cool_setpoint=%d",     actual_min_cool_setpoint);
      if (zbThermostat.getMaxCoolingSetpointLimit(    actual_max_cool_setpoint    ))    logEntry("  actual_max_cool_setpoint=%d",     actual_max_cool_setpoint);
      if (zbThermostat.getAbsMinCoolingSetpointLimit( actual_abs_min_cool_setpoint))    logEntry("  actual_abs_min_cool_setpoint=%d", actual_abs_min_cool_setpoint);
      if (zbThermostat.getAbsMaxCoolingSetpointLimit( actual_abs_max_cool_setpoint))    logEntry("  actual_abs_max_cool_setpoint=%d", actual_abs_max_cool_setpoint);
    }
  }

  ZigbeeAttribute<uint16_t> not_found(STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, 0x6666);
  ZigbeeAttribute<uint16_t> local_temperature(STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID);
  ZigbeeAttribute<int32_t> occupied_heating_setpoint(STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID);
  ZigbeeAttribute<uint8_t> ui_config_display_mode(STELPRO_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG, ESP_ZB_ZCL_ATTR_THERMOSTAT_UI_CONFIG_TEMPERATURE_DISPLAY_MODE_ID);

  not_found.setup();
  local_temperature.setup();
  occupied_heating_setpoint.setup();
  ui_config_display_mode.setup();

  logEntry("not_found.isInitialized()=%d", not_found.isInitialized());
  logEntry("local_temperature.isInitialized()=%d", local_temperature.isInitialized());
  logEntry("occupied_heating_setpoint.isInitialized()=%d", occupied_heating_setpoint.isInitialized());
  logEntry("ui_config_display_mode.isInitialized()=%d", ui_config_display_mode.isInitialized());
  logEntry("---");

  local_temperature.set(1500);
  //local_temperature.writeToZigbee();

  logEntry("not_found.get()=%d", not_found.get());
  logEntry("local_temperature.get()=%d", local_temperature.get());
  logEntry("occupied_heating_setpoint.get()=%d", occupied_heating_setpoint.get());
  logEntry("ui_config_display_mode.get()=%d", ui_config_display_mode.get());

  logEntry("%s", not_found.toString().c_str());
  logEntry("%s", local_temperature.toString().c_str());
  logEntry("%s", occupied_heating_setpoint.toString().c_str());
  logEntry("%s", ui_config_display_mode.toString().c_str());

  logEntry("};");

  //logEntry(">>> DEBUG: typeString<int8_t>()='%s'", typeString<int8_t>());
  //logEntry(">>> DEBUG: typeString<int16_t>()='%s'", typeString<int16_t>());
  //logEntry(">>> DEBUG: typeString<int32_t>()='%s'", typeString<int32_t>());
  //logEntry(">>> DEBUG: typeString<uint8_t>()='%s'", typeString<uint8_t>());
  //logEntry(">>> DEBUG: typeString<uint16_t>()='%s'", typeString<uint16_t>());
  //logEntry(">>> DEBUG: typeString<uint32_t>()='%s'", typeString<uint32_t>());
}

void simulateTemperature() {
  if (!tempSimulationUpdateTimer.hasTimedOut()) {
    return;
  }
  
  // reset timer for next iteration
  initTempSimulationUpdateTimer();

  // Get current values
  int16_t current_temp = 0;
  int16_t setpoint = 0;
  uint8_t system_mode = 0;
  zbThermostat.getLocalTemperature(current_temp);
  zbThermostat.getOccupiedHeatingSetpoint(setpoint);
  zbThermostat.getSystemMode(system_mode);

  // Simulate temperature based on heating demand
  if (system_mode == THERMOSTAT_SYSTEM_MODE_OFF) {
    // When off, temperature drifts toward room temp
    if (current_temp > SIMULATION_DEFAULT_ROOM_TEMPERATURE) {
      current_temp -= SIMULATION_TEMPERATURE_STEP_LOW;  // Cool down to room temperature
    } else if (current_temp < SIMULATION_DEFAULT_ROOM_TEMPERATURE) {
      current_temp += SIMULATION_TEMPERATURE_STEP_LOW;  // Warm up to room temperature
    }
  } else {
    // When heating, temperature moves toward setpoint
    if (current_temp < setpoint - SIMULATION_TEMPERATURE_DIFF_HIGH ) {
      current_temp += SIMULATION_TEMPERATURE_STEP_HIGH;   // Heat faster when far from setpoint
    } else if (current_temp < setpoint) {
      current_temp += SIMULATION_TEMPERATURE_STEP_LOW;    // Heat slower when close to setpoint
    } else if (current_temp > setpoint + SIMULATION_TEMPERATURE_DIFF_HIGH) {
      current_temp -= SIMULATION_TEMPERATURE_STEP_HIGH;   // Cool faster when too hot
    } else if (current_temp > setpoint) {
      current_temp -= SIMULATION_TEMPERATURE_STEP_LOW;    // Cool slower when close
    }
  }
  
  // Update temperature in thermostat (which updates internal EnergyCalculator)
  zbThermostat.setLocalTemperature(current_temp);
  
  // Get calculated values for logging
  uint8_t pi_heating_demand = 0;
  zbThermostat.getPIHeatingDemand(pi_heating_demand);
  uint16_t running_state = 0;
  zbThermostat.getRunningState(running_state);
  
  // Calculate power for display (internal calculator will compute accurately)
  int32_t power_watts = (STELPRO_MAX_POWER_WATTS * pi_heating_demand) / 100;
  
  logEntry("Simulation Update --> Temp: %.1f°C, Setpoint: %.1f°C, Demand: %d%%, State: %s, Power: %dW",
                current_temp / 100.0,
                setpoint / 100.0,
                pi_heating_demand,
                (running_state == THERMOSTAT_RUNNING_STATE_HEAT) ? "HEATING" : "IDLE",
                power_watts);


  //
  printAllAttributes();
}

// -------------------------------------------------------------------------
//                            Zigbee Callbacks
// -------------------------------------------------------------------------

void onIdentify(uint16_t time) {
  logEntry("Identify time changed to: %d seconds", time);
  if (time > 0) {
    // Enable identifyTimer timer. This will trigger the LED blinking feedback
    identifyTimer.setTimeOutTime(time * 1000);
    identifyTimer.reset();
  }
}

// Thermostat cluster
void onLocalTemperatureChange(int16_t temperature) {
  logEntry("Local Temperature changed from coordinator to: %.1f°C", temperature / 100.0);
}

void onOccupiedCoolSetpointChange(int16_t setpoint) {
  logEntry("Occupied Cool Setpoint changed from coordinator to: %.1f°C", setpoint / 100.0);
}

void onOccupiedHeatSetpointChange(int16_t setpoint) {
  logEntry("Occupied Heat Setpoint changed from coordinator to: %.1f°C", setpoint / 100.0);
}

void onControlSequenceOfOperationChange(uint8_t csop) {
  logEntry("Control Sequence Of Operation changed from coordinator to: %d", csop);
}

void onSystemModeChange(uint8_t mode) {
  logEntry("System mode changed from coordinator to: %s", (mode == THERMOSTAT_SYSTEM_MODE_HEAT) ? "HEAT" : "OFF");
}

// Thermostat UI cluster
void onDisplayModeChange(uint8_t mode) {
  logEntry("Display Mode changed from coordinator to: %d", mode);
}

void onKeypadLockoutChange(uint8_t lockout) {
  logEntry("Keypad Lockout changed from coordinator to: %d, %s", lockout, zb_zcl_thermostat_ui_config_keypad_lockout_to_string((zb_zcl_thermostat_ui_config_keypad_lockout_t)lockout));
}

// Thermostat cluster, additional attributes
void onRunningStateChange(uint16_t state) {
  logEntry("Running State changed from coordinator to: %d", state);
}

void onPIHeatingDemandChange(uint8_t demand) {
  logEntry("PI Heating Demand changed from coordinator to: %d", demand);
}

void onOutdoorTemperatureChange(int16_t temperature) {
  logEntry("Outdoor Temperature changed from coordinator to: %.1f°C", temperature / 100.0);
}

void onOccupancyChange(zb_uint8_t occupancy) {
  logEntry("Occupancy changed from coordinator to: %d", occupancy);
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
    logEntry(msg);
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
  
  Serial.println("========================================");
  Serial.println("  Stelpro HT402 Thermostat Emulator");
  Serial.println("========================================");
  Serial.println("Model: HT402 (Hilo)");
  Serial.println("Manufacturer: Stelpro");
  Serial.println("Endpoint: " + String(STELPRO_ENDPOINT));
  Serial.println("Type: Line-voltage heating thermostat");
  Serial.println("========================================");
  
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
  
  // Set callback functions for Zigbee
  // Thermostat cluster
  zbThermostat.onIdentify(onIdentify);
  zbThermostat.onLocalTemperatureChange(onLocalTemperatureChange);
  zbThermostat.onOccupiedCoolSetpointChange(onOccupiedCoolSetpointChange);
  zbThermostat.onOccupiedHeatSetpointChange(onOccupiedHeatSetpointChange);
  zbThermostat.onControlSequenceOfOperationChange(onControlSequenceOfOperationChange);
  zbThermostat.onSystemModeChange(onSystemModeChange);
  // Thermostat UI cluster
  zbThermostat.onDisplayModeChange(onDisplayModeChange);
  zbThermostat.onKeypadLockoutChange(onKeypadLockoutChange);
  // Thermostat cluster, additional attributes
  zbThermostat.onRunningStateChange(onRunningStateChange);
  zbThermostat.onPIHeatingDemandChange(onPIHeatingDemandChange);
  zbThermostat.onOutdoorTemperatureChange(onOutdoorTemperatureChange);
  zbThermostat.onOccupancyChange(onOccupancyChange);

  // Set manufacturer and model
  zbThermostat.setManufacturerAndModel(STELPRO_MANUFACTURER_NAME, STELPRO_MODEL_NAME);

  // DEBUG
  //zbThermostat.debugClusterList();
  //while(true) {}
  
  // Add endpoint to Zigbee Core
  Serial.println("Adding Zigbee Thermostat endpoint to Zigbee Core");
  Zigbee.addEndpoint(&zbThermostat);
  
  Serial.println("Starting Zigbee stack...");
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  }
  Serial.println("Zigbee stack ready.");
  
  // Init identifyTimer
  identifyTimer.setTimeOutTime(0);
  identifyTimer.reset();

  Serial.println("Connecting to network...");
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

  Serial.println("Connected to network!");

  // Connected - switch to blue pulse
  updateLEDStatus();
  blinker.loop();
  
  // Initialize simulation stuff
  zbThermostat.setOccupiedHeatingSetpoint(SIMULATION_TEMPERATURE_SETPOINT);

  // DEBUG
  printAllAttributes();
}

void loop() {
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
  zbThermostat.updateEnergy();  // Update energy simulation calculations and Zigbee attributes
  
  // Simulate temperature changes of zbThermostat
  if (Zigbee.connected()) {
    simulateTemperature();
  }
  
  delay(10);
}
