/**
 * @brief Daikin heatpump serial controller
 *
 * This sketch expose get/set commands through the UART1 serial port for controlling a Daikin Heatpump.
 *
 * Hardware:
 * - ESP32-C6 with onboard LED on GPIO8 (RGB LED)
 * - RGB LED on GPIO8
 * - BOOT button on GPIO9
 *
 * Arduino IDE Settings:
 * - Board: ESP32C6 Dev Module (the code might run on a 4MB board)
 *
 * Required Libraries:
 * - Button2 (install via Library Manager)
 * - SoftTimers (install via Library Manager)
 */

#include "RgbLedBlinker.h"
#include "Button2.h"
#include <SoftTimers.h>
#include "logging.h"
#include "scope_debugger.h"
#include "project_config.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "DaikinHTTP.h"
#include "WiFiConnectionManager.h"

#include "DaikinSerialApi.h"
#include "DaikinBridgeImpl.h"
#include "DaikinSerialListener.h"

#define UART0_TX 17
#define UART0_RX 16
#define UART1_TX 18
#define UART1_RX 19

// Pin definitions
#define LED_PIN RGB_BUILTIN   // RGB LED on ESP32-C6
#define BUTTON_PIN BOOT_PIN   // BOOT button on ESP32-C6

// Factory reset delay
#define FACTORY_RESET_LONG_CLICK_TIME 3 // in seconds, to press and hold button for factory reset

#define FORCE_REPORTING_INTERVAL          300000  //  5.0 minutes
#define DAIKIN_ACTIVITY_DURATION             800  //  0.8 seconds

// RGB LED blinker
RgbLedBlinker blinker;
enum LED_MODE {
  LED_MODE_OFF,
  LED_MODE_ACTIVITY,
  LED_MODE_WIFI_DISCONNECTED,
  LED_MODE_DAIKIN_OFFLINE,
  LED_MODE_CONNECTED,
};
LED_MODE previousLedMode = LED_MODE_OFF;

// Button handler
Button2 button;

// Update timers
SoftTimer forceReportingTimer;
SoftTimer activityTimer;

bool daikinOnline = false;

// -------------------------------------------------------------------------
//                          Daikin support section
// -------------------------------------------------------------------------
WiFiConnectionManager wifiManager;
DaikinBridgeImpl     bridge;
DaikinSerialListener listener;
DaikinHTTP daikin(SECRET_DAIKIN_HEATPUMP_IP);

#if 0
void daikinIncreaseTargetTempBy1() {
  // Pull to refresh latest data
  log_i("Pulling device info...");
  if (!daikin.pull()) {
    log_e("*** Failed to pull Daikin device info.");
    return;
  }
  log_i("pulled!");
  log_i("actual    payload=" + daikin.getControlInfoPayload().get());

  float target_temp = daikin.getTargetTemp();
  target_temp += 1.0;
  daikin.setTargetTemp(target_temp);
  log_i("temporary payload=" + daikin.getControlInfoPayload().get());

  log_i("Pushing new device info...");
  if (!daikin.push()) {
    log_e("*** Failed to push Daikin device info.");
    return;
  }
  log_i("pushed!");

  // Pull again to refresh changes
  log_i("Pulling device info...");
  if (!daikin.pull()) {
    log_e("*** Failed to pull Daikin device info.");
    return;
  }
  log_i("pulled!");
  log_i("actual    payload=" + daikin.getControlInfoPayload().get());
}
#endif // #if 0

bool daikinPullInfo() {
  log_i("Pull data from Daikin controller...");
  daikinOnline = false;
  if (!daikin.pull()) {
    log_e("*** Failed to pull Daikin device info.");
    return false;
  }
  daikinOnline = true;

  // start activity timer
  activityTimer.setTimeOutTime(DAIKIN_ACTIVITY_DURATION);
  activityTimer.reset();

  return true;
}

void daikinPrintInfo() {
  // Print payloads
  log_i("Daikin payloads: {");
  log_i("  Basic payload:   %s", daikin.getBasicInfoPayload().get()  .c_str());
  log_i("  Control payload: %s", daikin.getControlInfoPayload().get().c_str());
  log_i("  Sensor payload:  %s", daikin.getSensorInfoPayload().get() .c_str());
  log_i("}");

  DaikinSerialApi::daikin_status_info_t status = {};
  DaikinBridgeImpl::daikinHttp2DaikinStatusInfo(&daikin, &status);

  String status_desc = DaikinSerialApi::toString(&status, NUM_INDOOR_UNIT);
  log_i("Daikin heatpump attributes: %s", status_desc.c_str());
}

bool daikinPullAndPrintInfo() {
  if (!daikinPullInfo())
    return false;

  daikinPrintInfo();
  return true;
}

// -------------------------------------------------------------------------
//                          Reset/init functions
// -------------------------------------------------------------------------
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
}

void longClickDetected(Button2& btn) {
  //Serial.print("button long click detected!");
}

// -------------------------------------------------------------------------
//                          Temperature Simulation
// -------------------------------------------------------------------------

void daikinReportCheck() {
  // Make sure we do not call this function too often...
  if (forceReportingTimer.getTimeOutTime() != 0 && !forceReportingTimer.hasTimedOut()) {
    return; // too soon
  }
  // reset timer for next iteration timestamps
  forceReportingTimer.reset();

  daikinPullAndPrintInfo();
}

// -------------------------------------------------------------------------
//                            LED Status Update
// -------------------------------------------------------------------------
void updateLEDStatus() {
  LED_MODE newLedMode = LED_MODE_OFF;
  const char * msg = "";

  // Check device's state to know how the LED should behave
  if (!wifiManager.isHealthy()) {
    if (!wifiManager.isConnected())
      msg = "WiFi Disconnected - LED set to fast ORANGE blink";
    else
      msg = "WiFi Unstable - LED set to fast ORANGE blink";
    newLedMode = LED_MODE_WIFI_DISCONNECTED;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_ORANGE);
  } else if (!daikinOnline) {
    msg = "DAIKIN Heatpump offline - LED set to fast PURPLE blink";
    newLedMode = LED_MODE_DAIKIN_OFFLINE;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_PURPLE);
  } else if (activityTimer.getTimeOutTime() != 0 && !activityTimer.hasTimedOut()) { // if active and not timed out
    msg = "Activity - LED set to fast BLUE blink";
    newLedMode = LED_MODE_ACTIVITY;
    blinker.set(RgbLedBlinker::MODE_BLINK_FAST, RgbLedBlinker::COLOR_BLUE);
  } else {
    msg = "Connected - LED set to slow GREEN pulse";
    newLedMode = LED_MODE_CONNECTED;
    blinker.set(RgbLedBlinker::MODE_PULSE_ONCE_PER_30_SECONDS, RgbLedBlinker::COLOR_GREEN);
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
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  // Initialize RGB LED blinker
  blinker.setup(LED_PIN);
  blinker.set(RgbLedBlinker::MODE_OFF, RgbLedBlinker::COLOR_BLACK);
  blinker.loop(); // force the LED to turn off

  // Wait up to 3s for serial
  while (!Serial  && millis() < 3000);
  while (!Serial1 && millis() < 3000);

  bridge.begin(&daikin);
  listener.begin(Serial1, &bridge);

  log_i("========================================");
  log_i("  Daiking Controller");
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
  
  // Initialize force reporting timer
  initForceReportingTimer();

  wifiManager.setup();

  log_i("Connecting to wifi...");
  size_t loop_count = 0;
  while (wifiManager.getState() != WiFiConnectionManager::WIFI_STATE::HEALTHY) {
    loop_count++;
    if (loop_count % 25 == 0) {
      log_i("WiFi state: %s", WiFiConnectionManager::toString(wifiManager.getState()));
    }

    wifiManager.loop(); // call loop to allow wifiManager to detect and change state

    // Keep LED blinking during connection
    updateLEDStatus();
    blinker.loop();

    delay(100);
  }
  log_i("Connected to wifi!");

  //log_i("Waiting 5000 ms...");
  //delay(5000);

  // Init identifyTimer
  activityTimer.setTimeOutTime(0);
  activityTimer.reset();

  // Force a pull from Daikin controller immediately to force `daikinOnline` flag to true ASAP.
  daikinPullAndPrintInfo();

  // Connected - for LED to show updated status
  updateLEDStatus();
  blinker.loop();
}

void loop() {
  static uint32_t loop_count = 0;
  loop_count++;

  // Disable activityTimer, after timed out
  if (activityTimer.getTimeOutTime() != 0 && activityTimer.hasTimedOut()) { // if active and has timed out
    activityTimer.setTimeOutTime(0);
    activityTimer.reset();
  }

  // Refresh WiFi connection state
  wifiManager.loop();

  // Update LED status based on connection state
  updateLEDStatus();

  // Update all components
  blinker.loop();
  button.loop();

  // Print Daikin info periodicaly.
  daikinReportCheck();

  // Parse any incomming messages
  listener.loop();
  
  delay(10);
}
