#include "logging.h"
#include "Arduino.h"
#include "esp_zigbee_core.h"

static const char *TAG = "STELPRO_HT402";

void logEntry(const char* fmt, ...) {
  char buffer[2048];
  va_list args;
  
  // Convert arguments to string
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  
  // Log with ESP API
  ESP_LOGI(TAG, "%s", buffer);

  // Also output on the serial port. 
  Serial.println(buffer);
}

void logError(esp_err_t err, const char * file, int line) {
  if (err == ESP_OK)
    return;
  const char * err_name = esp_err_to_name(err);
  logEntry("Error in file %s, line %d: 0x%04x, %s", file, line, err, err_name);
}

