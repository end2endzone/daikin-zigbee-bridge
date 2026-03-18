#pragma once

#include "Arduino.h"
#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

static String strformat(const char* fmt, ...) {
  va_list args;

  // Calculate the required length
  va_start(args, fmt);
  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (len < 0) return String("");

  // Allocate bufferfor the string + null terminator
  char* buffer = (char*)malloc(len + 1);
  if (!buffer) return String("");

  // Run vsnprintf again to fill buffer
  va_start(args, fmt);
  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  // Return buffer as String
  String output(buffer, (unsigned int)len);
  free(buffer);
  return output;
}

/*
 * Converts an input buffer to its hexadecimal string representation.
 *
 * Output format : "0x<hex digits>" (e.g. "0xDEADBEEF")
 *
 * Returns true  when the full representation fits input buffer.
 * Returns false otherwise.
 */
static bool toHex(const void* input, size_t input_size, char* buffer, size_t buffer_size)
{
  if ( buffer == NULL || buffer_size == 0 )
    return false;
  if ( buffer_size > 0 )
    buffer[0] = '\0'; // EMPTY output by default

  // NULL input
  if (input == nullptr) {
    // Can we return "NULL" as a string?
    if (buffer_size >= 5) {
      strcpy(buffer, "NULL");
    }

    // Empty string
    return true;
  }

  // Validate empty input
  if ( input_size == 0 )
    return true;

  // Compute minimum buffer size:
  //  "0x"                        : 2 chars
  //  2 hex digits per input byte : input_size * 2 chars
  //  '\0'                        : 1 char
  size_t required_size = input_size * 2 + 3;   // "0x" + hex digits + '\0'
  if ( buffer_size < required_size )
    return false;

  const uint8_t* bytes = (const uint8_t*)input;

  buffer[0] = '0';
  buffer[1] = 'x';

  size_t pos = 2; // start at index 2 since `0x` was already inserted

  // for each byte in the input buffer
  for ( size_t i = 0; i < input_size; i++ ) {
    static const char hex[] = "0123456789ABCDEF";
    buffer[pos+0] = hex[(bytes[i] >> 4) & 0x0F];  // high nibble
    buffer[pos+1] = hex[bytes[i] & 0x0F];         // low nibble
    pos += 2;
  }

  buffer[pos] = '\0';
  return true;
}

/*
 * Converts an input buffer to its binary string representation.
 *
 * Output format : "b<binary digits>" (e.g. "b0110100001101001")
 *
 * Returns true  when the full representation fits in buffer.
 * Returns false otherwise.
 */
static bool toBin(const void* input, size_t input_size, char* buffer, size_t buffer_size)
{
  if ( buffer == NULL || buffer_size == 0 )
    return false;
  if ( buffer_size > 0 )
    buffer[0] = '\0'; // EMPTY output by default

  // NULL input
  if ( input == nullptr )
  {
    // Can we return "NULL" as a string?
    if ( buffer_size >= 5 )
    {
      strcpy(buffer, "NULL");
    }

    // Empty string
    return true;
  }

  // Validate empty input
  if ( input_size == 0 )
    return true;

  // Compute minimum buffer size:
  //  "b"                   : 1 char
  //  8 bits per input byte : input_size * 8 chars
  //  '\0'                  : 1 char
  //  Total                 : input_size * 8 + 2
  //
  size_t required_size = input_size * 8 + 2;   // "b" + bits + '\0'
  if ( buffer_size < required_size )
    return false;

  const uint8_t* bytes = (const uint8_t*)input;

  buffer[0] = 'b';

  size_t pos = 1; // start at index 2 since `b` was already inserted

  for ( size_t i = 0; i < input_size; i++ ) {
    for ( int bit = 7; bit >= 0; bit-- ) {
      buffer[pos] = (char)('0' + ((bytes[i] >> bit) & 1));
      pos++;
    }
  }

  buffer[pos] = '\0';
  return true;
}

static void endian_swap(void* buffer, size_t buffer_size) {
  if (buffer == NULL || buffer_size <= 1)
    return;

  uint8_t* bytes = (uint8_t*)buffer;

  for (size_t i = 0; i < buffer_size / 2; i++) {
    uint8_t tmp = bytes[i];
    bytes[i] = bytes[buffer_size - 1 - i];
    bytes[buffer_size - 1 - i] = tmp;
  }
}

static void lowercase(void* buffer, size_t buffer_size) {
  if (buffer == NULL || buffer_size == 0)
    return;

  uint8_t* bytes = (uint8_t*)buffer;

  for (size_t i = 0; i < buffer_size; i++) {
    if (bytes[i] >= 'A' && bytes[i] <= 'Z') {
      bytes[i] = bytes[i] + ('a' - 'A');
    }
  }
}

static void lowercase_str(char* str) {
  if (str == NULL)
    return;

  char* p = nullptr;
  for (p = str; *p != '\0'; p++) {
    if (*p >= 'A' && *p <= 'Z') {
      *p = *p + ('a' - 'A');
    }
  }
}
