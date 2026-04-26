#pragma once

#include "Zigbee.h"
#include "esp32-hal-log.h"
#include "esp_zigbee_core.h"

/*
ESP32 log functions:
  ERROR:    log_e(format, ...)
  WARNING:  log_w(format, ...)
  INFO:     log_i(format, ...)
  DEBUG:    log_d(format, ...)
  VERBOSE:  log_v(format, ...)
*/

static const char* logBaseFileName(const char* file_path) {
  if (!file_path) {
    return nullptr;
  }

  // Find the last occurrence of either slash type
  const char* slash1 = strrchr(file_path, '/');
  const char* slash2 = strrchr(file_path, '\\');

  const char* last_slash = slash1 > slash2 ? slash1 : slash2;

  // If no slash found, the whole string is the filename
  return last_slash ? last_slash + 1 : file_path;
}

static inline void logError(esp_err_t err) {
  if (err == ESP_OK)
    return;
  const char * err_name = esp_err_to_name(err);
  log_e("*** Error: 0x%04x, %s", err, err_name);
}
