#include <WiFi.h>
#include <HTTPClient.h>

/*
class EnumHelper {
public:
  struct EnumHelper {
    const char * name;  // description
    const char * payload; // serialized payload value
  }

  static int getName(const int value, const EnumHelper * helpers, size_t helper_count, int default_value) {
    return helpers[value].name;
  }

  static int deserialize(const String &payload, const EnumHelper * helpers, size_t helper_count, int default_value) {
    for (int i = 0; i < helper_count; i++) {
      if (payload == helpers[i].payload) return i;
    }
    return default_value;
  }

  static String serialize(const String &payload, const EnumHelper * helpers, size_t helper_count, int default_value) {
    for (int i = 0; i < helper_count; i++) {
      if (payload == helpers[i].payload) return i;
    }
    return default_value;
  }
};
*/

class DaikinHTTP {
public:

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

  // Lookup table: maps key enums to string key in payload
  static constexpr char* KeyNames[] = {
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

  static String toStringFan(int value) {
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

  static String serializeFan(int value) {
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

  static int deserializeFan(const String &value) {
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
  // Swing directions
  //------------------------------------------------------
  enum Swing {
    SWING_OFF,
    SWING_VERTICAL,
    SWING_HORIZONTAL,
    SWING_BOTH,
    SWING_ERROR
  };

  static String toStringSwing(int value) {
    switch (value) {
      case SWING_OFF:         return "Off";
      case SWING_VERTICAL:    return "Vertical";
      case SWING_HORIZONTAL:  return "Horizontal";
      case SWING_BOTH:        return "Both";
      default:                return "Error";
    }
  }

  static String serializeSwing(int value) {
    switch (value) {
      case SWING_OFF:         return "0";
      case SWING_VERTICAL:    return "1";
      case SWING_HORIZONTAL:  return "2";
      case SWING_BOTH:        return "3";
      default:                return "-";
    }
  }

  static int deserializeSwing(const String &value) {
    if (value == "0") return SWING_OFF;
    if (value == "1") return SWING_VERTICAL;
    if (value == "2") return SWING_HORIZONTAL;
    if (value == "3") return SWING_BOTH;
    return SWING_ERROR;
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
  // Payload helper functions
  //------------------------------------------------------
private:
  static bool payloadFindKeyValue(const String &payload, const String &key, int & start_index, int & end_index) {
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

  static String payloadGetValueString(const String &payload, const String &key) {
    //Serial.println("searching for key:" + key);
    //Serial.println("payload=" + payload);
    int start_index = -1;
    int end_index = -1;
    bool found = payloadFindKeyValue(payload, key, start_index, end_index);
    if (!found) {
      //Serial.println("key not found!");
      return "";
    }

    // Extract payload value
    String value = payload.substring(start_index, end_index);
    return value;
  }

  static void payloadSetValueString(String &payload, const String &key, const String &value) {
    // Search for the exiting key
    int start_index = -1;
    int end_index = -1;
    bool found = payloadFindKeyValue(payload, key, start_index, end_index);
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

  static float payloadGetValueFloat(const String &payload, const String &key) {
    String tmp = payloadGetValueString(payload, key);
    return tmp.length() ? tmp.toFloat() : NAN;
  }

  static void payloadSetValueFloat(String &payload, const String &key, const float &value) {
    String tmp = String(value);
    payloadSetValueString(payload, key, tmp);
  }


  //------------------------------------------------------
  // REST Api functions
  //------------------------------------------------------
private:
  String ip;

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

  bool getControlInfo(String &response) {
    return httpGet("/aircon/get_control_info", response);
  }

  bool getSensorInfo(String &response) {
    return httpGet("/aircon/get_sensor_info", response);
  }

  //------------------------------------------------------
  // Payload public utility functions
  //------------------------------------------------------
public:
  static String getPayloadString(const String &payload, Key key) {
    const String name = toString(key);
    String value = payloadGetValueString(payload, name);
    return value;
  }

  static void setPayloadString(String &payload, Key key, const String &value) {
    const String name = toString(key);
    payloadSetValueString(payload, name, value);
  }

  static float getPayloadFloat(const String &payload, Key key) {
    const String name = toString(key);
    float value = payloadGetValueFloat(payload, name);
    return value;
  }

  static void setPayloadFloat(String &payload, Key key, const float &value) {
    const String name = toString(key);
    payloadSetValueFloat(payload, name, value);
  }

  //------------------------------------------------------
  // Parsing utility functions
  //------------------------------------------------------
public:
  // Control parsers
  static Mode parseMode(const String &payload) {
    String tmp = getPayloadString(payload, KEY_OPERATION_MODE);
    Mode output = static_cast<Mode>(deserializeMode(tmp));
    return output;
  }

  static FanRate parseFanRate(const String &payload) {
    String tmp = getPayloadString(payload, KEY_FAN_RATE);
    FanRate output = static_cast<FanRate>(deserializeFan(tmp));
    return output;
  }

  static Swing parseSwing(const String &payload) {
    String tmp = getPayloadString(payload, KEY_FAN_DIRECTION);
    Swing output = static_cast<Swing>(deserializeSwing(tmp));
    return output;
  }

  static Preset parsePreset(const String &payload) {
    String tmp = getPayloadString(payload, KEY_PRESET_MODE);
    Preset output = static_cast<Preset>(deserializePreset(tmp));
    return output;
  }

  static float parseTargetTemp(const String &payload) {
    float output = getPayloadFloat(payload, KEY_TARGET_TEMP);
    return output;
  }

  // Sensor parsers
  static float parseIndoorTemp(const String &payload) {
    float output = getPayloadFloat(payload, KEY_INDOOR_TEMP);
    return output;
  }

  static float parseOutdoorTemp(const String &payload) {
    float output = getPayloadFloat(payload, KEY_OUTDOOR_TEMP);
    return output;
  }
  
};
