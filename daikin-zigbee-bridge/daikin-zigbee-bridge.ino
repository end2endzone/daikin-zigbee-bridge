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
  
  // Get basic info
  if (daikin.fetchBasicInfo()) {
    Serial.print("DEBUG: Basic payload:"); Serial.println(daikin.getBasicInfoPayload().get());

    String device_name = daikin.getDeviceName();
    Serial.print("Device name:  "); Serial.println(device_name);
  }
  
  // Get control info
  if (daikin.fetchControlInfo()) {
    Serial.print("DEBUG: Control payload:"); Serial.println(daikin.getControlInfoPayload().get());

    DaikinHTTP::Mode mode = daikin.getMode();
    DaikinHTTP::FanRate fan = daikin.getFanRate();
    DaikinHTTP::Swing swing = daikin.getSwing();
    DaikinHTTP::Preset preset = daikin.getPreset();
    float target_temp = daikin.getTargetTemp();

    Serial.println("Mode: " + DaikinHTTP::toString(mode));
    Serial.println("Fan: " + DaikinHTTP::toString(fan));
    Serial.println("Swing: " + DaikinHTTP::toString(swing));
    Serial.println("Preset: " + DaikinHTTP::toString(preset));
    Serial.print("Target Temp:  "); Serial.println(target_temp);
  } else {
    Serial.println("Failed to get control info.");
  }

  // Get sensor info
  if (daikin.fetchSensorInfo()) {
    Serial.print("DEBUG: Sensor payload:"); Serial.println(daikin.getSensorInfoPayload().get());

    float indoor = daikin.getIndoorTemp();
    float outdoor = daikin.getOutdoorTemp();

    Serial.print("Indoor Temp:  "); Serial.println(indoor);
    Serial.print("Outdoor Temp: "); Serial.println(outdoor);
  } else {
    Serial.println("Failed to get sensor info.");
  }

  delay(10000);
}
