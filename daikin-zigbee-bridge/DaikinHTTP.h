#include <WiFi.h>
#include <HTTPClient.h>

class DaikinHTTP {
public:
  // Operating modes
  enum Mode {
    MODE_AUTO,
    MODE_HEATING,
    MODE_COOLING,
    MODE_DEHUMIDIFIER,
    MODE_FAN,
    MODE_ERROR
  };

  // Fan rates
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

  // Swing directions
  enum Swing {
    SWING_OFF,
    SWING_VERTICAL,
    SWING_HORIZONTAL,
    SWING_BOTH,
    SWING_ERROR
  };

  // Preset modes
  enum Preset {
    PRESET_NONE,
    PRESET_AWAY,
    PRESET_ECO,
    PRESET_BOOST,
    PRESET_ERROR
  };

  // Enum for all known keys in Daikin HTTP payloads
  enum Key {
    KEY_RETURN_STATUS,        // "ret" - return status
    KEY_OPERATION_MODE,       // "mode" - operating mode (heat/cool/etc.)
    KEY_FAN_RATE,             // "f_rate" - fan speed
    KEY_FAN_DIRECTION,        // "f_dir" - swing direction
    KEY_PRESET_MODE,          // "adv" - preset (eco/away/boost)
    KEY_INDOOR_TEMP,          // "htemp" - indoor temperature
    KEY_OUTDOOR_TEMP,         // "otemp" - outdoor temperature
    KEY_INDOOR_HUMIDITY,      // "hhum" - indoor humidity
    KEY_TARGET_TEMP,          // "stemp" - setpoint temperature
    KEY_UNKNOWN               // fallback for unknown keys
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

  // Generic getter using Key enum
  String getValue(const String &payload, Key key) {
    return "";
  }

  // Generic setter using Key enum
  static void setValue(String &payload, Key key, const String &value) {
  }

private:
  String ip;

  // Lookup table: maps key enums to string key in payload
  static constexpr const char* KeyNames[] = {
    "ret",       // KEY_RETURN_STATUS
    "mode",      // KEY_OPERATION_MODE
    "f_rate",    // KEY_FAN_RATE
    "f_dir",     // KEY_FAN_DIRECTION
    "adv",       // KEY_PRESET_MODE
    "htemp",     // KEY_INDOOR_TEMP
    "otemp",     // KEY_OUTDOOR_TEMP
    "hhum",      // KEY_INDOOR_HUMIDITY
    "stemp",     // KEY_TARGET_TEMP
    "unknown"    // KEY_UNKNOWN
  };

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

public:
  DaikinHTTP(String ipAddr) : ip(ipAddr) {}

  bool getControlInfo(String &response) {
    return httpGet("/aircon/get_control_info", response);
  }

  bool getSensorInfo(String &response) {
    return httpGet("/aircon/get_sensor_info", response);
  }

  static bool findValue(const String &payload, const String &key, int & start_index, int & end_index) {
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

  static String getValue(const String &payload, const String &key) {
    int start_index = -1;
    int end_index = -1;
    //Serial.println("searching for key:" + key);
    //Serial.println("payload=" + payload);
    bool found = findValue(payload, key, start_index, end_index);
    if (!found) {
      //Serial.println("key not found!");
    }
    if (found) {
      String value = payload.substring(start_index, end_index);
      //Serial.println("found value:" + value);
      return value;
    }
    return "";
  }

  static void setValue(String &payload, const String &key, const String &value) {
    // Search for the exiting key
    String pattern = key + "=";
    int start = payload.indexOf(pattern);
    if (start == -1) {
      // Key not found, append at end
      if (!payload.endsWith(",")) payload += ",";
      payload += key + "=" + value;
      return;
    }

    // Key is found. Compute exiting key length
    start += key.length() + 1;
    int end = payload.indexOf(",", start);
    if (end == -1) end = payload.length();

    // Override previous key with new value
    payload = payload.substring(0, start) + value + payload.substring(end);
  }

  // Control parsers
  static Mode parseMode(const String &controlPayload) {
    String val = getValue(controlPayload, "mode");
    if (val == "0") return MODE_AUTO;
    if (val == "2") return MODE_DEHUMIDIFIER;
    if (val == "3") return MODE_COOLING;
    if (val == "4") return MODE_HEATING;
    if (val == "6") return MODE_FAN;
    return MODE_ERROR;
  }

  static FanRate parseFanRate(const String &controlPayload) {
    String val = getValue(controlPayload, "f_rate");
    if (val == "A") return FAN_AUTO;
    if (val == "B") return FAN_QUIET;
    if (val == "3") return FAN_LEVEL1;
    if (val == "4") return FAN_LEVEL2;
    if (val == "5") return FAN_LEVEL3;
    if (val == "6") return FAN_LEVEL4;
    if (val == "7") return FAN_LEVEL5;
    return FAN_ERROR;
  }

  static Swing parseSwing(const String &controlPayload) {
    String val = getValue(controlPayload, "f_dir");
    if (val == "0") return SWING_OFF;
    if (val == "1") return SWING_VERTICAL;
    if (val == "2") return SWING_HORIZONTAL;
    if (val == "3") return SWING_BOTH;
    return SWING_ERROR;
  }

  static Preset parsePreset(const String &controlPayload) {
    String val = getValue(controlPayload, "adv");
    if (val == "0") return PRESET_NONE;
    if (val == "1") return PRESET_AWAY;
    if (val == "2") return PRESET_ECO;
    if (val == "3") return PRESET_BOOST;
    return PRESET_ERROR;
  }

  // Sensor parsers
  static float parseIndoorTemp(const String &sensorPayload) {
    String val = getValue(sensorPayload, "htemp");
    return val.length() ? val.toFloat() : NAN;
  }

  static float parseOutdoorTemp(const String &sensorPayload) {
    String val = getValue(sensorPayload, "otemp");
    return val.length() ? val.toFloat() : NAN;
  }

  // String conversion helpers
  static String toString(Mode mode) {
    switch (mode) {
      case MODE_AUTO: return "Auto";
      case MODE_HEATING: return "Heating";
      case MODE_COOLING: return "Cooling";
      case MODE_DEHUMIDIFIER: return "Dehumidifier";
      case MODE_FAN: return "Fan";
      default: return "Error";
    }
  }

  static String toString(FanRate fan) {
    switch (fan) {
      case FAN_AUTO: return "Auto";
      case FAN_QUIET: return "Quiet";
      case FAN_LEVEL1: return "Level 1";
      case FAN_LEVEL2: return "Level 2";
      case FAN_LEVEL3: return "Level 3";
      case FAN_LEVEL4: return "Level 4";
      case FAN_LEVEL5: return "Level 5";
      default: return "Error";
    }
  }

  static String toString(Swing swing) {
    switch (swing) {
      case SWING_OFF: return "Off";
      case SWING_VERTICAL: return "Vertical";
      case SWING_HORIZONTAL: return "Horizontal";
      case SWING_BOTH: return "Both";
      default: return "Error";
    }
  }

  static String toString(Preset preset) {
    switch (preset) {
      case PRESET_NONE: return "None";
      case PRESET_AWAY: return "Away";
      case PRESET_ECO: return "Eco";
      case PRESET_BOOST: return "Boost";
      default: return "Error";
    }
  }
};
