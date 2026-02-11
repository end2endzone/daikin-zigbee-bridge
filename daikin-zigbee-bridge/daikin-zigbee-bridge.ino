#ifdef ENABLE_DAIKINHTTP
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "DaikinHTTP.h"
#endif // ENABLE_DAIKINHTTP


#ifdef ENABLE_DAIKINHTTP
// -------------------------------------------------------------------------
//                          Daikin support section
// -------------------------------------------------------------------------
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
//                          Main sketch section
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

#ifdef ENABLE_DAIKINHTTP
  daikinSetup();
#endif // ENABLE_DAIKINHTTP
}

void loop() {
}
