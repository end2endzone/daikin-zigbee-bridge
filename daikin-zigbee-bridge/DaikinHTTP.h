#include <WiFi.h>
#include <HTTPClient.h>

class DaikinHttpPayload {
private:
  String payload;

public:
  DaikinHttpPayload() : payload() {}
  DaikinHttpPayload(const String& payload) : payload(payload) {}

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

  bool findKeyValue(const String &key, int & start_index, int & end_index) const {
    start_index = -1;
    end_index = -1;

    // Search for `,key=` first
    String pattern = "," + key + "=";
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
    int end = payload.indexOf(",", start);
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
      // Key not found, append at end
      if (!payload.endsWith(",")) payload += ",";
      payload += key + "=" + value;
      return;
    }

    // Key is found.
    // Override previous key with new value
    payload = payload.substring(0, start_index) + value + payload.substring(end_index);
  }

  float getValueFloat(const String &key) const {
    String tmp = getValueString(key);
    return tmp.length() ? tmp.toFloat() : NAN;
  }

  void setValueFloat(const String &key, const float &value) {
    String tmp = String(value);
    setValueString(key, tmp);
  }

};

class DaikinHTTP {
public:

  // Enum for all known keys in Daikin HTTP payloads
  enum Key {
    KEY_RETURN_STATUS,        // "ret" - return status
    KEY_DEVICE_POWER,         // "pow" - from Control payload
    KEY_OPERATION_MODE,       // "mode" - operating mode (heat/cool/etc.)
    KEY_FAN_RATE,             // "f_rate" - fan speed
    KEY_FAN_DIRECTION,        // "f_dir" - FanDir direction
    KEY_PRESET_MODE,          // "adv" - preset (eco/away/boost)
    KEY_INDOOR_TEMP,          // "htemp" - indoor temperature
    KEY_OUTDOOR_TEMP,         // "otemp" - outdoor temperature
    KEY_INDOOR_HUMIDITY,      // "hhum" - indoor humidity
    KEY_TARGET_TEMP,          // "stemp" - setpoint temperature
    KEY_DEVICE_NAME,          // "name" - from basic info
    KEY_UNKNOWN               // fallback for unknown keys
  };

  // Lookup table: maps key enums to string key in payload
  static constexpr char* KeyNames[] = {
    "ret",      // KEY_RETURN_STATUS
    "pow",      // KEY_DEVICE_POWER
    "mode",     // KEY_OPERATION_MODE
    "f_rate",   // KEY_FAN_RATE
    "f_dir",    // KEY_FAN_DIRECTION
    "adv",      // KEY_PRESET_MODE
    "htemp",    // KEY_INDOOR_TEMP
    "otemp",    // KEY_OUTDOOR_TEMP
    "hhum",     // KEY_INDOOR_HUMIDITY
    "stemp",    // KEY_TARGET_TEMP
    "name",     // KEY_DEVICE_NAME
    "unknown"   // KEY_UNKNOWN
  };

  static const char* toString(Key key) {
    return KeyNames[key];
  }

  static Key fromString(const String& str) {
    for (int i = 0; i < KEY_UNKNOWN; i++) {
      if (str == KeyNames[i]) return static_cast<Key>(i);
    }
    return KEY_UNKNOWN;
  }


  //------------------------------------------------------
  // Power
  //------------------------------------------------------
  enum Power {
    POWER_OFF,
    POWER_ON,
    POWER_ERROR
  };

  static String toStringPower(int value) {
    switch (value) {
      case POWER_OFF:     return "Off";
      case POWER_ON:      return "On";
      default:            return "Error";
    }
  }

  static String serializePower(int value) {
    switch (value) {
      case POWER_OFF:     return "0";
      case POWER_ON:      return "1";
      default:            return "-";
    }
  }

  static int deserializePower(const String &value) {
    if (value == "0") return POWER_OFF;
    if (value == "1") return POWER_ON;
    return POWER_ERROR;
  }


  //------------------------------------------------------
  // Operating modes
  //------------------------------------------------------
  enum Mode {
    MODE_AUTO,
    MODE_DEHUMIDIFIER,
    MODE_COOLING,
    MODE_HEATING,
    MODE_FAN,
    MODE_ERROR
  };

  static String toStringMode(int value) {
    switch (value) {
      case MODE_AUTO:             return "Auto";
      case MODE_DEHUMIDIFIER:     return "Dehumidifier";
      case MODE_COOLING:          return "Cooling";
      case MODE_HEATING:          return "Heating";
      case MODE_FAN:              return "Fan";
      default:                    return "Error";
    }
  }

  static String serializeMode(int value) {
    switch (value) {
      case MODE_AUTO:             return "0";
      case MODE_DEHUMIDIFIER:     return "2";
      case MODE_COOLING:          return "3";
      case MODE_HEATING:          return "4";
      case MODE_FAN:              return "6";
      default:                    return "-";
    }
  }

  static int deserializeMode(const String &value) {
    if (value == "0") return MODE_AUTO;
    if (value == "2") return MODE_DEHUMIDIFIER;
    if (value == "3") return MODE_COOLING;
    if (value == "4") return MODE_HEATING;
    if (value == "6") return MODE_FAN;
    return MODE_ERROR;
  }


  //------------------------------------------------------
  // Fan Rate
  //------------------------------------------------------
  enum FanRate {
    FAN_AUTO,
    FAN_QUIET,
    FAN_LEVEL1,
    FAN_LEVEL2,
    FAN_LEVEL3,
    FAN_LEVEL4,
    FAN_LEVEL5,
    FAN_ERROR
  };

  static String toStringFanRate(int value) {
    switch (value) {
      case FAN_AUTO:    return "Auto";
      case FAN_QUIET:   return "Quiet";
      case FAN_LEVEL1:  return "Level 1";
      case FAN_LEVEL2:  return "Level 2";
      case FAN_LEVEL3:  return "Level 3";
      case FAN_LEVEL4:  return "Level 4";
      case FAN_LEVEL5:  return "Level 5";
      default:          return "Error";
    }
  }

  static String serializeFanRate(int value) {
    switch (value) {
      case FAN_AUTO:    return "A";
      case FAN_QUIET:   return "B";
      case FAN_LEVEL1:  return "3";
      case FAN_LEVEL2:  return "4";
      case FAN_LEVEL3:  return "5";
      case FAN_LEVEL4:  return "6";
      case FAN_LEVEL5:  return "7";
      default:          return "Error";
    }
  }

  static int deserializeFanRate(const String &value) {
    if (value == "A") return FAN_AUTO;
    if (value == "B") return FAN_QUIET;
    if (value == "3") return FAN_LEVEL1;
    if (value == "4") return FAN_LEVEL2;
    if (value == "5") return FAN_LEVEL3;
    if (value == "6") return FAN_LEVEL4;
    if (value == "7") return FAN_LEVEL5;
    return FAN_ERROR;
  }


  //------------------------------------------------------
  // Fan Direction
  //------------------------------------------------------
  enum FanDir {
    FanDir_OFF,
    FanDir_VERTICAL,
    FanDir_HORIZONTAL,
    FanDir_BOTH,
    FanDir_ERROR
  };

  static String toStringFanDir(int value) {
    switch (value) {
      case FanDir_OFF:         return "Off";
      case FanDir_VERTICAL:    return "Vertical";
      case FanDir_HORIZONTAL:  return "Horizontal";
      case FanDir_BOTH:        return "Both";
      default:                return "Error";
    }
  }

  static String serializeFanDir(int value) {
    switch (value) {
      case FanDir_OFF:         return "0";
      case FanDir_VERTICAL:    return "1";
      case FanDir_HORIZONTAL:  return "2";
      case FanDir_BOTH:        return "3";
      default:                return "-";
    }
  }

  static int deserializeFanDir(const String &value) {
    if (value == "0") return FanDir_OFF;
    if (value == "1") return FanDir_VERTICAL;
    if (value == "2") return FanDir_HORIZONTAL;
    if (value == "3") return FanDir_BOTH;
    return FanDir_ERROR;
  }


  //------------------------------------------------------
  // Preset modes
  //------------------------------------------------------
  enum Preset {
    PRESET_NONE,
    PRESET_AWAY,
    PRESET_ECO,
    PRESET_BOOST,
    PRESET_ERROR
  };

  static String toStringPreset(int value) {
    switch (value) {
      case PRESET_NONE:   return "None";
      case PRESET_AWAY:   return "Away";
      case PRESET_ECO:    return "Eco";
      case PRESET_BOOST:  return "Boost";
      default:            return "Error";
    }
  }

  static String serializePreset(int value) {
    switch (value) {
      case PRESET_NONE:   return "0";
      case PRESET_AWAY:   return "1";
      case PRESET_ECO:    return "2";
      case PRESET_BOOST:  return "3";
      default:            return "-";
    }
  }

  static int deserializePreset(const String &value) {
    if (value == "0") return PRESET_NONE;
    if (value == "1") return PRESET_AWAY;
    if (value == "2") return PRESET_ECO;
    if (value == "3") return PRESET_BOOST;
    return PRESET_ERROR;
  }

  //------------------------------------------------------
  // REST Api functions
  //------------------------------------------------------
private:
  String ip;
  DaikinHttpPayload basicInfo;
  DaikinHttpPayload controlInfo;
  DaikinHttpPayload sensorInfo;

  bool httpGet(const String &endpoint, String &response) {
    HTTPClient http;
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

  //------------------------------------------------------
  // Payload public utility functions
  //------------------------------------------------------
public:
  DaikinHTTP(String ipAddr) : ip(ipAddr) {}

  bool fetchBasicInfo() {
    controlInfo.clear();
    bool success = httpGet("/common/basic_info", basicInfo.get());
    return success;
  }

  bool fetchControlInfo() {
    controlInfo.clear();
    bool success = httpGet("/aircon/get_control_info", controlInfo.get());
    return success;
  }

  bool fetchSensorInfo() {
    sensorInfo.clear();
    bool success = httpGet("/aircon/get_sensor_info", sensorInfo.get());
    return success;
  }

  bool update() {
    if (!fetchBasicInfo())
      return false;
    if (!fetchControlInfo())
      return false;
    if (!fetchSensorInfo())
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

  //------------------------------------------------------
  // Payload getter/setter functions
  //------------------------------------------------------
private:
  DaikinHttpPayload* findPayloadForKey(Key key) {
    switch(key) {
      case KEY_RETURN_STATUS:     return NULL; // both has it
      case KEY_DEVICE_POWER:      return &controlInfo;
      case KEY_OPERATION_MODE:    return &controlInfo;
      case KEY_FAN_RATE:          return &controlInfo;
      case KEY_FAN_DIRECTION:     return &controlInfo;
      case KEY_PRESET_MODE:       return &controlInfo;
      case KEY_INDOOR_TEMP:       return &sensorInfo;
      case KEY_OUTDOOR_TEMP:      return &sensorInfo;
      case KEY_INDOOR_HUMIDITY:   return &controlInfo;
      case KEY_TARGET_TEMP:       return &controlInfo;
      case KEY_DEVICE_NAME:       return &basicInfo;
      case KEY_UNKNOWN:           return NULL;
      default:                    return NULL;
    }
  }

  String getKeyString(Key key) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return "";
    const String keyName = toString(key);
    String value = payload->getValueString(keyName);
    return value;
  }

  void setKeyString(Key key, const String &value) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return;
    const String keyName = toString(key);
    payload->setValueString(keyName, value);
  }

  float getKeyFloat(Key key) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return NAN;
    const String keyName = toString(key);
    float value = payload->getValueFloat(keyName);
    return value;
  }

  void setKeyFloat(Key key, const float &value) {
    DaikinHttpPayload* payload = findPayloadForKey(key);
    if (payload == NULL) return;
    const String keyName = toString(key);
    payload->setValueFloat(keyName, value);
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
      char accumulator[3] = {0};  // holds two hex digits + null terminator
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
    String tmp = getKeyString(KEY_DEVICE_NAME);
    String decoded = decodePercentEncoded(tmp);
    return decoded;
  }

  // Control info parsers
  Power getPower() {
    String tmp = getKeyString(KEY_DEVICE_POWER);
    Power output = static_cast<Power>(deserializePower(tmp));
    return output;
  }

  Mode getMode() {
    String tmp = getKeyString(KEY_OPERATION_MODE);
    Mode output = static_cast<Mode>(deserializeMode(tmp));
    return output;
  }

  FanRate getFanRate() {
    String tmp = getKeyString(KEY_FAN_RATE);
    FanRate output = static_cast<FanRate>(deserializeFanRate(tmp));
    return output;
  }

  FanDir getFanDir() {
    String tmp = getKeyString(KEY_FAN_DIRECTION);
    FanDir output = static_cast<FanDir>(deserializeFanDir(tmp));
    return output;
  }

  Preset getPreset() {
    String tmp = getKeyString(KEY_PRESET_MODE);
    Preset output = static_cast<Preset>(deserializePreset(tmp));
    return output;
  }

  float getTargetTemp() {
    float output = getKeyFloat(KEY_TARGET_TEMP);
    return output;
  }

  // Sensor info parsers
  float getIndoorTemp() {
    float output = getKeyFloat(KEY_INDOOR_TEMP);
    return output;
  }

  float getOutdoorTemp() {
    float output = getKeyFloat(KEY_OUTDOOR_TEMP);
    return output;
  }

  //------------------------------------------------------
  // toString() functions
  //------------------------------------------------------
public:
  static String toString(Power value) {
    return toStringPower(value);
  }

  static String toString(Mode value) {
    return toStringMode(value);
  }

  static String toString(FanRate value) {
    return toStringFanRate(value);
  }

  static String toString(FanDir value) {
    return toStringFanDir(value);
  }

  static String toString(Preset value) {
    return toStringPreset(value);
  }
  
};
