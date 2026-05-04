#pragma once

#include <stdint.h>
#include <inttypes.h>
#include "SerialInterface.h"

class Protocol
{
public:
  typedef void (*MessageCallback)(uint8_t msg_id, const uint8_t *msg_payload, uint16_t msg_payload_size);

  static constexpr uint8_t SYNC_SIGNATURE_SIZE = 7;
  static constexpr uint8_t SYNC_SIGNATURE[SYNC_SIGNATURE_SIZE] = {0xAA, 0x55, 'D', 'Z', 'A', '4', 'H'}; // Daikin-Zigbee-Adaptor-4-Hilo

  // Default timeout used by synchronous query methods.
  static constexpr unsigned long DEFAULT_TIMEOUT_MS = 5000;

  // Result codes returned by the synchronous query methods.
  enum class QueryResult
  {
    SUCCESS = 0, // A valid, matching response was received.
    TIMEOUT,     // No response was received within the allotted time.
    UNKNOWN,     // An unexpected / unrecoverable error occurred.
  };

  typedef struct message_info_s {
    uint8_t id;           // unique id, indicates the type of payload – serialized
    uint8_t *payload;     // payload actual bytes – serialized
    uint16_t payload_size;// size of payload in bytes – NOT serialized
    uint8_t crc;          // checksum for id + payload bytes – serialized
  } message_info_t;

  typedef struct message_header_s {
    uint8_t id;
    uint8_t payload; // first byte of the payload bytes
  } message_header_t;

  typedef struct message_footer_s {
    uint8_t crc;
  } message_footer_t;

  Protocol(SerialInterface *serial, uint8_t *buffer, uint16_t buffer_size);

  // --------------------------------------------------------------------------
  // Asynchronous / callback-based API
  // --------------------------------------------------------------------------

  // Register a callback that is invoked from loop() whenever a complete,
  // CRC-valid message is received.
  void onMessageReceived(MessageCallback cb);

  // Must be called repeatedly from the Arduino loop().  Reads all currently
  // available bytes, advances the state machine, and fires the registered
  // callback for each complete message.
  void loop();

  // --------------------------------------------------------------------------
  // Synchronous API
  // --------------------------------------------------------------------------

  // Block until one complete, CRC-valid message is received or timeout_ms
  // milliseconds have elapsed.
  //
  // On SUCCESS response is filled with the received message fields.
  // The payload pointer inside response points into the internal receive buffer
  // and is valid only until the next read operation on this Protocol instance.
  //
  // NOTE: Do NOT call loop() while using the synchronous API; both share the
  //       same internal state machine and receive buffer.
  QueryResult readNextMessage(message_info_s *response,
                              unsigned long timeout_ms = DEFAULT_TIMEOUT_MS);

  // Send a request message and block until any response is received, or
  // timeout_ms milliseconds have elapsed.
  //
  // It is the caller's responsibility to inspect response_payload to determine
  // whether the received message is the expected one.
  //
  // On SUCCESS the output arguments are set exactly as in readNextMessage().
  QueryResult QueryMessageResponse(uint8_t request_id,
                                   const uint8_t *request_payload,
                                   uint16_t request_payload_size,
                                   uint8_t **response_payload,
                                   uint16_t *response_payload_size,
                                   unsigned long timeout_ms = DEFAULT_TIMEOUT_MS);

  // --------------------------------------------------------------------------
  // Utility
  // --------------------------------------------------------------------------

  size_t sendMessage(uint8_t msg_id, const uint8_t *msg_payload, uint16_t msg_payload_size);
  size_t flushReadBuffer(unsigned long timeout_time);

private:
  SerialInterface * port;

  uint8_t sync_window[SYNC_SIGNATURE_SIZE] = {0};

  enum ParseState
  {
    WAIT_SYNC,
    WAIT_LEN1,
    WAIT_LEN2,
    WAIT_PAYLOAD
  };

  ParseState state;

  uint16_t read_msg_length;         // serialized length of the message currently being received
  uint16_t last_complete_msg_length;// serialized length of the last fully-received message

  uint8_t *read_msg_buffer;
  uint16_t read_msg_buffer_size;
  uint16_t read_msg_index_pos;

  MessageCallback callback = nullptr;

  bool checkSync();
  uint8_t calcCRC(const uint8_t *data, uint16_t length);
  uint8_t calcCRC(uint8_t msg_id, const uint8_t *data, uint16_t length);

  // Feed one byte into the state machine.
  // Returns true when a complete CRC-valid message has been placed in
  // read_msg_buffer. When true is returned, the caller may inspect
  // read_msg_buffer[0..last_complete_msg_length-1].
  // Returns false otherwise.
  bool processByte(uint8_t b);

  // Extract message fields from the receive buffer after processByte()
  // returns true. Fills response and returns SUCCESS, or returns UNKNOWN on a
  // structural error.
  QueryResult extractLastMessage(message_info_s *response);
};
