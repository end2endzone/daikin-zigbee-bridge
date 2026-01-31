#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "DaikinHTTP.h"

DaikinHTTP daikin(SECRET_DAIKIN_HEATPUMP_IP);

void setup() {
  Serial.begin(115200);
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  /*
  TODO: Add a begin() function that check if it can communicate with the heatpump.
  if (daikin.begin()) {
    Serial.println("Connected to Daikin heatpump!");
  } else {
    Serial.println("Failed to connect to Daikin heatpump.");
  }
  */
}

void loop() {
  
  if (!daikin.update()) {
    Serial.println("Failed to update device info.");
    delay(10000);
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
    DaikinHTTP::Mode mode = daikin.getMode();
    DaikinHTTP::FanRate fan = daikin.getFanRate();
    DaikinHTTP::FanDir FanDir = daikin.getFanDir();
    DaikinHTTP::Preset preset = daikin.getPreset();
    float target_temp = daikin.getTargetTemp();

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

  delay(10000);
}
