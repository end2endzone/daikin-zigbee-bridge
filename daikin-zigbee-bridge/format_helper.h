#pragma once

#include "Arduino.h"
#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

static String format(const char* fmt, ...) {
  char buffer[2048];
  va_list args;
  
  // Convert arguments to string
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  return buffer;
}

static void toHex(uint8_t byte, char *out) {
  static const char hex[] = "0123456789ABCDEF";

  out[0] = hex[(byte >> 4) & 0x0F];  // high nibble
  out[1] = hex[byte & 0x0F];         // low nibble
}

void toHex(const void *input, size_t input_size, char *out) {
  if (out == nullptr) {
    return;
  }
  if (input == nullptr || input_size == 0) {
    out[0] = '\0';
    return;
  }

  const uint8_t *bytes = (const uint8_t *)input;

  for (size_t i = 0; i < input_size; i++) {
    toHex(bytes[i], out + (i * 2));
  }

  out[input_size * 2] = '\0';
}
