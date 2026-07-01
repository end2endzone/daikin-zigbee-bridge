#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include "DaikinEnums.h"

#define INVALID_VALUE_FLOAT NAN
#define INVALID_VALUE_INT 0xFFFF

class DaikinHttpPayload {
private:
  String payload;
  String separator; // usualy `,` but sometimes `&`

public:
  DaikinHttpPayload() :
    payload(),
    separator(",")
    {}
  DaikinHttpPayload(const String& payload) :
    payload(payload),
    separator(",")
    {}
  DaikinHttpPayload(const String& payload, const String& separator) :
    payload(payload),
    separator(separator)
    {}

  void clear() {
    payload.clear();
  }

  const String& get() const {
    return payload;
  }

  String& get() {
    return payload;
  }

  void set(const String& value) {
    payload = value;
  }

  const String& getSeparator() const {
    return separator;
  }

  void setSeparator(const String& value) {
    separator = value;
  }

  bool findKeyValue(const String &key, int & start_index, int & end_index) const {
    start_index = -1;
    end_index = -1;

    // Search for `,key=` first
    String pattern = separator + key + "=";
    int start = payload.indexOf(pattern);
    if (start == -1) {
      // Pattern `,key=` not found. Maybe the key is the first key in the payload.
      // Search for `key=`.
      pattern = key + "=";
      start = payload.indexOf(pattern);
      if (start == -1) return false; // error
    }
    else
    {
      start++; // skip `,`
    }

    // Key found, extract value boundaries
    start += key.length() + 1; // skip `key=`
    int end = payload.indexOf(separator, start);
    if (end == -1) end = payload.length();

    // Success
    start_index = start;
    end_index = end;
    return true;
  }

  String getValueString(const String &key) const {
    //Serial.println("searching for key:" + key);
    //Serial.println("payload=" + payload);
    int start_index = -1;
    int end_index = -1;
    bool found = findKeyValue(key, start_index, end_index);
    if (!found) {
      //Serial.println("key not found!");
      return "";
    }

    // Extract payload value
    String value = payload.substring(start_index, end_index);
    return value;
  }

  void setValueString(const String &key, const String &value) {
    // Search for the exiting key
    int start_index = -1;
    int end_index = -1;
    bool found = findKeyValue(key, start_index, end_index);
    if (!found) {
      //Serial.println("setValueString(): key not found:" + key);
      // Key not found, append at end
      if (!payload.isEmpty() && !payload.endsWith(separator)) payload += separator;
      payload += key + "=" + value;
      //Serial.println("setValueString(): payload set to:" + payload);
      return;
    }

    // Key is found.
    // Override previous key with new value
    //Serial.println("setValueString(): key not found:" + key);
    payload = payload.substring(0, start_index) + value + payload.substring(end_index);
    //Serial.println("setValueString(): payload set to:" + payload);
  }

  float getValueFloat(const String &key) const {
    String tmp = getValueString(key);
    return tmp.length() ? tmp.toFloat() : INVALID_VALUE_FLOAT;
  }

  void setValueFloat(const String &key, const float &value) {
    String tmp = String(value);
    setValueString(key, tmp);
  }

  int getValueInt(const String &key) const {
    String tmp = getValueString(key);
    return tmp.length() ? tmp.toInt() : INVALID_VALUE_INT;
  }

  void setValueInt(const String &key, const int &value) {
    String tmp = String(value);
    setValueString(key, tmp);
  }

};

class DaikinHTTP {
private:
  String ip;
  DaikinHttpPayload basicInfo;
  DaikinHttpPayload controlInfo;
  DaikinHttpPayload sensorInfo;
  uint16_t          timeout; // milliseconds

  static constexpr const char* MandatoryKeyNames[] = {
    "pow",      // KEY_DEVICE_POWER
    "mode",     // KEY_OPERATION_MODE
    "stemp",    // KEY_TARGET_TEMP
    "shum",     // KEY_TARGET_HUMIDITY
    "f_rate",   // KEY_FAN_RATE
    "f_dir",    // KEY_FAN_DIRECTION
  };
  static const int MandatoryKeyNamesCount = sizeof(MandatoryKeyNames) / sizeof(MandatoryKeyNames[0]);

  bool httpGet(const String &endpoint, String &response) {
    HTTPClient http;
    http.setTimeout(timeout);
    String url = "http://" + ip + endpoint;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == 200) {
      response = http.getString();
      http.end();
      return true;
    }
    http.end();
    return false;
  }

  bool httpPost(const String &endpoint, const String &payload) {
    HTTPClient http;
    http.setTimeout(timeout);
    String url = "http://" + ip + endpoint;
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(payload);
    if (httpCode == 200) {
      http.end();
      return true;
    }
    http.end();
    return false;
  }

  //------------------------------------------------------
  // Payload public utility functions
  //------------------------------------------------------
public:
  DaikinHTTP(String ipAddr) :
    ip(ipAddr),
    timeout((uint16_t)-1)
  {
  }

  DaikinHTTP() :
    ip("0.0.0.0"),
    timeout((uint16_t)-1)
  {
  }

  void setIP(String ip) {
    this->ip = ip;
  }

  bool pullBasicInfo() {
    controlInfo.clear();
    bool success = httpGet("/common/basic_info", basicInfo.get());
    return success;
  }

  bool pullControlInfo() {
    controlInfo.clear();
    bool success = httpGet("/aircon/get_control_info", controlInfo.get());
    return success;
  }

  bool pushControlInfo() {
    // The European versions of the Wifi Controller Unit BRP069Axx (where xx is 41, 42, 43 or 45) and BRP069Bxx are expecting a GET request with parameters encoded into the URL.
    // For example http://192.168.1.100/aircon/set_control_info?pow=1&mode=4&stemp=26&shum=0&f_rate=7&f_dir=0

    // Copy mandatory keys from controlInfo to an output payload
    DaikinHttpPayload outputInfo("", "&"); // use `&` as separator since this payload will be added to the get url
    //Serial.println("payload=" + outputInfo.get());
    for(int i=0; i<MandatoryKeyNamesCount; i++) {
      const String key = MandatoryKeyNames[i];

      // Read previous pull payload
      String value = controlInfo.getValueString(key);
      if (value == "") return false;

      // Encode value into output payload
      outputInfo.setValueString(key, value);
      //Serial.println("payload=" + outputInfo.get());
    }

    String endpoint = "/aircon/set_control_info?" + outputInfo.get();
    //Serial.println("endpoint=" + outputInfo.get());
    
    String response;
    bool success = httpGet(endpoint, response);
    return success;
  }

  bool pullSensorInfo() {
    sensorInfo.clear();
    bool success = httpGet("/aircon/get_sensor_info", sensorInfo.get());
    return success;
  }

  bool pull() {
    if (!pullBasicInfo())
      return false;
    if (!pullControlInfo())
      return false;
    if (!pullSensorInfo())
      return false;
    return true;
  }

  bool push() {
    if (!pushControlInfo())
      return false;
    return true;
  }

  const DaikinHttpPayload& getBasicInfoPayload() const {
    return basicInfo;
  }

  const DaikinHttpPayload& getControlInfoPayload() const {
    return controlInfo;
  }

  const DaikinHttpPayload& getSensorInfoPayload() const {
    return sensorInfo;
  }

  void setTimeout(uint16_t timeout) {
    this->timeout = timeout;
  }

  uint16_t getTimeout() const {
    return timeout;
  }

  //------------------------------------------------------
  // Payload getter/setter functions
  //------------------------------------------------------
private:
  DaikinHttpPayload* findPayloadForKey(DaikinEnums::Key key) {
    switch(key) {
      case DaikinEnums::KEY_RETURN_STATUS:     return NULL; // both has it
      case DaikinEnums::KEY_DEVICE_POWER:      return &controlInfo;
      case DaikinEnums::KEY_OPERATION_MODE:    return &controlInfo;
      case DaikinEnums::KEY_FAN_RATE:          return &controlInfo;
      case DaikinEnums::KEY_FAN_DIRECTION:     return &controlInfo;
      case DaikinEnums::KEY_PRESET_AWAY:       return &controlInfo;
      case DaikinEnums::KEY_PRESET_ECONO:      return &controlInfo;
      case DaikinEnums::KEY_PRESET_POWERFUL:   return &controlInfo;
      case DaikinEnums::KEY_INDOOR_TEMP:       return &sensorInfo;
      case DaikinEnums::KEY_OUTDOOR_TEMP:      return &sensorInfo;
      case DaikinEnums::KEY_COMP_FREQ:         return &sensorInfo;
      case DaikinEnums::KEY_INDOOR_HUMIDITY:   return &controlInfo;
      case DaikinEnums::KEY_TARGET_TEMP:       return &controlInfo;
      case DaikinEnums::KEY_DEVICE_NAME:       return &basicInfo;
      case DaikinEnums::KEY_UNKNOWN:           return NULL;
      default:                    return NULL;
    }
  }

  String getKeyString(DaikinEnums::Key key) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return "";
    const String keyName = DaikinEnums::toString(key);
    String value = payload->getValueString(keyName);
    return value;
  }

  void setKeyString(DaikinEnums::Key key, const String &value) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return;
    const String keyName = DaikinEnums::toString(key);
    payload->setValueString(keyName, value);
  }

  float getKeyFloat(DaikinEnums::Key key) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return INVALID_VALUE_FLOAT;
    const String keyName = DaikinEnums::toString(key);
    float value = payload->getValueFloat(keyName);
    return value;
  }

  void setKeyFloat(DaikinEnums::Key key, const float &value) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return;
    const String keyName = DaikinEnums::toString(key);
    payload->setValueFloat(keyName, value);
  }

  int getKeyInt(DaikinEnums::Key key) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return INVALID_VALUE_INT;
    const String keyName = DaikinEnums::toString(key);
    int value = payload->getValueInt(keyName);
    return value;
  }

  void setKeyInt(DaikinEnums::Key key, const int &value) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return;
    const String keyName = DaikinEnums::toString(key);
    payload->setValueInt(keyName, value);
  }

  //------------------------------------------------------
  // Name decoding utility functions
  //------------------------------------------------------
private:
  int hexDigitToInt(char c) {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
      if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
      return 0; // fallback
  }

  String decodePercentEncoded(const String& encoded) {
      String decoded;
      char accumulator[3] = {};  // holds two hex digits + null terminator
      int accIndex = 0; // accumulator index where the next insertation must be

      // for each characters in the encoded string
      for (size_t i = 0; i < encoded.length(); ++i) {
          char c = encoded[i];
          if (c == '%') {
              // skip the '%' itself
              continue;
          } else {
              // collect hex digits into accumulator
              accumulator[accIndex] = c;
              accIndex++;

              // if the accumulator is full
              if (accIndex == 2) {
                  // convert two hex digits into a character
                  int high = hexDigitToInt(accumulator[0]);
                  int low  = hexDigitToInt(accumulator[1]);
                  char decodedChar = static_cast<char>((high << 4) | low);
                  decoded += decodedChar;

                  // reset accumulator
                  accIndex = 0;
                  accumulator[0] = 0;
                  accumulator[1] = 0;
              }
          }
      }
      return decoded;
  }

  //------------------------------------------------------
  // Parsing utility functions
  //------------------------------------------------------
public:

  // Basic info parsers
  String getDeviceName() {
    String tmp = getKeyString(DaikinEnums::KEY_DEVICE_NAME);
    String decoded = decodePercentEncoded(tmp);
    return decoded;
  }

  // Control info parsers
  DaikinEnums::Power getPower() {
    String tmp = getKeyString(DaikinEnums::KEY_DEVICE_POWER);
    DaikinEnums::Power output = static_cast<DaikinEnums::Power>(DaikinEnums::deserializePower(tmp));
    return output;
  }

  void setPower(DaikinEnums::Power value) {
    String tmp = DaikinEnums::serializePower(value);
    setKeyString(DaikinEnums::KEY_DEVICE_POWER, tmp);
  }

  DaikinEnums::Mode getMode() {
    String tmp = getKeyString(DaikinEnums::KEY_OPERATION_MODE);
    DaikinEnums::Mode output = static_cast<DaikinEnums::Mode>(DaikinEnums::deserializeMode(tmp));
    return output;
  }

  void setMode(DaikinEnums::Mode value) {
    String tmp = DaikinEnums::serializeMode(value);
    setKeyString(DaikinEnums::KEY_OPERATION_MODE, tmp);
  }

  DaikinEnums::FanRate getFanRate() {
    String tmp = getKeyString(DaikinEnums::KEY_FAN_RATE);
    DaikinEnums::FanRate output = static_cast<DaikinEnums::FanRate>(DaikinEnums::deserializeFanRate(tmp));
    return output;
  }

  void setFanRate(DaikinEnums::FanRate value) {
    String tmp = DaikinEnums::serializeFanRate(value);
    setKeyString(DaikinEnums::KEY_FAN_RATE, tmp);
  }

  DaikinEnums::FanDir getFanDir() {
    String tmp = getKeyString(DaikinEnums::KEY_FAN_DIRECTION);
    DaikinEnums::FanDir output = static_cast<DaikinEnums::FanDir>(DaikinEnums::deserializeFanDir(tmp));
    return output;
  }

  void setFanDir(DaikinEnums::FanDir value) {
    String tmp = DaikinEnums::serializeFanDir(value);
    setKeyString(DaikinEnums::KEY_FAN_DIRECTION, tmp);
  }

  bool isPresetAway() {
    int output = getKeyInt(DaikinEnums::KEY_PRESET_AWAY);
    if (output == INVALID_VALUE_INT) return false;
    return output != 0;
  }
 
  bool isPresetEcono() {
    int output = getKeyInt(DaikinEnums::KEY_PRESET_ECONO);
    if (output == INVALID_VALUE_INT) return false;
    return output != 0;
  }
 
  bool isPresetPowerful() {
    int output = getKeyInt(DaikinEnums::KEY_PRESET_POWERFUL);
    if (output == INVALID_VALUE_INT) return false;
    return output != 0;
  }
 
  DaikinEnums::Preset getPreset() {
    if (isPresetAway())         return DaikinEnums::Preset::PRESET_AWAY;
    if (isPresetEcono())        return DaikinEnums::Preset::PRESET_ECO;
    if (isPresetPowerful())     return DaikinEnums::Preset::PRESET_BOOST;
    return DaikinEnums::Preset::PRESET_NONE;
  }

  float getTargetTemp() {
    float output = getKeyFloat(DaikinEnums::KEY_TARGET_TEMP);
    return output;
  }

  void setTargetTemp(const float &value) {
    setKeyFloat(DaikinEnums::KEY_TARGET_TEMP, value);
  }

  // Sensor info parsers
  float getIndoorTemp() {
    float output = getKeyFloat(DaikinEnums::KEY_INDOOR_TEMP);
    return output;
  }

  float getOutdoorTemp() {
    float output = getKeyFloat(DaikinEnums::KEY_OUTDOOR_TEMP);
    return output;
  }
  
  int getCompressorFrequency() {
    int output = getKeyInt(DaikinEnums::KEY_COMP_FREQ);
    return output;
  }
};
