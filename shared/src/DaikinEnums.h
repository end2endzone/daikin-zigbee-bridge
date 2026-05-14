#pragma once

#include <Arduino.h>

class DaikinEnums {
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
  static constexpr const char* KeyNames[] = {
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
