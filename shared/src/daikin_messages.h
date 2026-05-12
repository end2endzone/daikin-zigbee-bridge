#pragma once

// Shared message definitions for the serial protocol between the two ESP32 devices.
// This file is expected to be used by both projects.
// It defines the requests and responses messages that are exchanged with 
// SerialTransfer communication library.
//
// Wire layout
// -----------
// Every exchange is a request / response pair distinguished by a packetID byte
// that SerialTransfer carries in its frame header:
//
//   Controller --> Listener   packetID = PACKET_ID_<NAME>_REQUEST
//   Listener   --> Controller packetID = PACKET_ID_<NAME>_RESPONSE
//
// All structs are marked __attribute__((packed)) so that the compiler adds no
// padding; this guarantees identical memory layout on both ends.

#include <stdint.h>
#include <stddef.h>


// ---------------------------------------------------------------------------
// Packet IDs
//
// Each request/response pair occupies two consecutive IDs so that the
// controller can derive the expected response ID as (request_id + 1).
// ---------------------------------------------------------------------------
#define PACKET_ID_GET_IP_ADDRESS_REQUEST    ((uint8_t)1)
#define PACKET_ID_GET_IP_ADDRESS_RESPONSE   ((uint8_t)2)
#define PACKET_ID_SET_TARGET_TEMP_REQUEST   ((uint8_t)3)
#define PACKET_ID_SET_TARGET_TEMP_RESPONSE  ((uint8_t)4)
#define PACKET_ID_GET_STATUS_REQUEST        ((uint8_t)5)
#define PACKET_ID_GET_STATUS_RESPONSE       ((uint8_t)6)


// ---------------------------------------------------------------------------
// Internal structs
//
// Request / response structs exchanged witht he SerialTransfer library.
// 
// All structs are marked __attribute__((packed)) so that the compiler adds no padding.
// This guarantees identical memory layout on both ends.
// 
// Empty requests have a mandatory single reserved byte. The SerialTransfer library 
// requires at least one payload byte to build a valid frame.
// -----------------------------------------------------------------------------

typedef struct __attribute__((packed)) get_ip_address_request_s {
  uint8_t reserved;   // always 0x00. I do not know if SerialTransfer library supports 0-bytes messages (where `sizeof(get_status_request_s)==0`)
} get_ip_address_request_t;

typedef struct __attribute__((packed)) get_ip_address_response_s {
  DaikinSerialApi::ApiResult  result;
  DaikinSerialApi::daikin_ip_address_t addr;
} get_ip_address_response_t;

typedef struct __attribute__((packed)) set_target_temp_request_s {
  uint16_t temperature;   // units: 0.01 °C  (e.g. 2100 = 21.00 °C)
} set_target_temp_request_t;

typedef struct __attribute__((packed)) set_target_temp_response_s {
  DaikinSerialApi::ApiResult result;
} set_target_temp_response_t;

typedef struct __attribute__((packed)) get_status_request_s {
  uint8_t reserved;   // always 0x00. I do not know if SerialTransfer library supports 0-bytes messages (where `sizeof(get_status_request_s)==0`)
} get_status_request_t;

typedef struct __attribute__((packed)) get_status_response_s {
  DaikinSerialApi::ApiResult      result;
  DaikinSerialApi::daikin_status_info_t status;
} get_status_response_t;
