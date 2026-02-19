#include "logging.h"
#include "Arduino.h"
#include "esp_zigbee_core.h"

static const char *TAG = "STELPRO_HT402";

const char* logBaseFileName(const char* file_path) {
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

void logEntry(const char* fmt, ...) {
  va_list args;

  // Calculate the required length
  va_start(args, fmt);
  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (len < 0) return;

  // Allocate bufferfor the string + null terminator
  char* buffer = (char*)malloc(len + 1);
  if (!buffer) return;

  // Run vsnprintf again to fill buffer
  va_start(args, fmt);
  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  // Log with ESP API
  ESP_LOGI(TAG, "%s", buffer);

  // Also output on the serial port. 
  Serial.println(buffer);

  free(buffer);
}

void logError(esp_err_t err, const char * file, int line) {
  if (err == ESP_OK)
    return;
  const char * err_name = esp_err_to_name(err);
  logEntry("Error in file %s, line %d: 0x%04x, %s", file, line, err, err_name);
}

