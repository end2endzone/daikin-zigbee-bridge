#pragma once

#include "Arduino.h"
#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

#include <cstdint>

template <typename T>
const char* typeString() {
  return "<unknown>";
}

// ---- Unsigned integer specializations ----

template <>
inline const char* typeString<uint8_t>() {
  return "uint8_t";
}

template <>
inline const char* typeString<uint16_t>() {
  return "uint16_t";
}

template <>
inline const char* typeString<uint32_t>() {
  return "uint32_t";
}

// ---- Signed integer specializations ----

template <>
inline const char* typeString<int8_t>() {
  return "int8_t";
}

template <>
inline const char* typeString<int16_t>() {
  return "int16_t";
}

template <>
inline const char* typeString<int32_t>() {
  return "int32_t";
}

// ---- zigbee types specializations ----

template <>
inline const char* typeString<esp_zb_int24_s>() {
  return "esp_zb_int24_s";
}

template <>
inline const char* typeString<esp_zb_uint24_s>() {
  return "esp_zb_uint24_s";
}

template <>
inline const char* typeString<esp_zb_int48_s>() {
  return "esp_zb_int48_s";
}

template <>
inline const char* typeString<esp_zb_uint48_s>() {
  return "esp_zb_uint48_s";
}

