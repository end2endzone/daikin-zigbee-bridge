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

static void toHex(const void *input, size_t input_size, char *out) {
  if (out == nullptr) {
    return;
  }
  if (input == nullptr || input_size == 0) {
    out[0] = '\0';
    return;
  }

  const uint8_t *bytes = (const uint8_t *)input;

  // For each byte in the output string
  for (size_t i = 0; i < input_size; i++) {

    // Reverse byte read order (to be able to print the value and get the expected 0x12345678)
    uint8_t b = bytes[input_size - 1 - i];

    // Encode
    toHex(b, out + (i * 2));
  }  

  out[input_size * 2] = '\0';
}

static void toHexSafe(const void *input, size_t input_size, char* buffer, size_t buffer_size) {
  if (buffer == nullptr)
    return;
  if (buffer_size > 0)
    buffer[0] = '\0'; // EMPTY output by default
  
  // Validate empty input
  if (input_size == 0)
    return;
  
  // NULL input
  if (input == nullptr) {
    if (buffer_size >= 5) {
      strcpy(buffer, "NULL");
    } else if (buffer_size >= 4) {
      strcpy(buffer, "NUL");
    } else {
      // EMPTY output
    }
    return;
  }

  // Validate minimum size for `0x0`
  if (buffer_size < 4)
    return; // EMPTY output

  // print the hex representation of input
  buffer[0] = '0';
  buffer[1] = 'x';
  toHex(input, input_size, &buffer[2]);
}


