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
  String payload;

  // --- Get control info ---
  if (daikin.getControlInfo(payload)) {
    Serial.println("DEBUG: Control payload:");
    Serial.println(payload);

    DaikinHTTP::Mode mode = daikin.parseMode(payload);
    DaikinHTTP::FanRate fan = daikin.parseFanRate(payload);
    DaikinHTTP::Swing swing = daikin.parseSwing(payload);
    DaikinHTTP::Preset preset = daikin.parsePreset(payload);

    Serial.println("Mode: " + DaikinHTTP::toString(mode));
    Serial.println("Fan: " + DaikinHTTP::toString(fan));
    Serial.println("Swing: " + DaikinHTTP::toString(swing));
    Serial.println("Preset: " + DaikinHTTP::toString(preset));
  } else {
    Serial.println("Failed to get control info.");
  }

  // --- Get sensor info ---
  if (daikin.getSensorInfo(payload)) {
    Serial.println("DEBUG: Sensor payload:");
    Serial.println(payload);

    float indoor = daikin.parseIndoorTemp(payload);
    float outdoor = daikin.parseOutdoorTemp(payload);

    Serial.print("Indoor Temp:  "); Serial.println(indoor);
    Serial.print("Outdoor Temp: "); Serial.println(outdoor);
  } else {
    Serial.println("Failed to get sensor info.");
  }

  delay(10000);
}
