#pragma once

#include "Arduino.h"
#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

#include <cstdint>

enum TypeSign {
  TYPE_SIGN_UNKNOWN = 0,
  TYPE_SIGN_UNSIGNED,
  TYPE_SIGN_SIGNED
};

static const char * toString(TypeSign type_sign) {
  switch (type_sign) {
    case TYPE_SIGN_UNSIGNED:  return "unsigned";
    case TYPE_SIGN_SIGNED:    return "signed";
    default:
    case TYPE_SIGN_UNKNOWN:   return "unknown";
  };
}

template <typename T> inline const char* typeString() { return "<unknown>"; }
template <typename T> inline TypeSign typeSign() { return TYPE_SIGN_UNKNOWN; }

// ---- Unsigned integer specializations ----

template <> inline const char* typeString<uint8_t>()    { return "uint8_t";  }
template <> inline const char* typeString<uint16_t>()   { return "uint16_t"; }
template <> inline const char* typeString<uint32_t>()   { return "uint32_t"; }

template <> inline TypeSign typeSign<uint8_t>()   { return TYPE_SIGN_UNSIGNED; }
template <> inline TypeSign typeSign<uint16_t>()  { return TYPE_SIGN_UNSIGNED; }
template <> inline TypeSign typeSign<uint32_t>()  { return TYPE_SIGN_UNSIGNED; }

// ---- Signed integer specializations ----

template <> inline const char* typeString<int8_t>()   { return "int8_t";  }
template <> inline const char* typeString<int16_t>()  { return "int16_t"; }
template <> inline const char* typeString<int32_t>()  { return "int32_t"; }

template <> inline TypeSign typeSign<int8_t>()  { return TYPE_SIGN_SIGNED; }
template <> inline TypeSign typeSign<int16_t>() { return TYPE_SIGN_SIGNED; }
template <> inline TypeSign typeSign<int32_t>() { return TYPE_SIGN_SIGNED; }

// ---- zigbee types specializations ----

template <> inline const char* typeString<esp_zb_uint24_s>()  { return "esp_zb_uint24_s"; }
template <> inline const char* typeString<esp_zb_uint48_s>()  { return "esp_zb_uint48_s"; }
template <> inline TypeSign typeSign<esp_zb_uint24_s>() { return TYPE_SIGN_SIGNED; }
template <> inline TypeSign typeSign<esp_zb_uint48_s>() { return TYPE_SIGN_SIGNED; }

template <> inline const char* typeString<esp_zb_int24_s>()   { return "esp_zb_int24_s";  }
template <> inline const char* typeString<esp_zb_int48_s>()   { return "esp_zb_int48_s";  }
template <> inline TypeSign typeSign<esp_zb_int24_s>()  { return TYPE_SIGN_SIGNED; }
template <> inline TypeSign typeSign<esp_zb_int48_s>()  { return TYPE_SIGN_SIGNED; }
