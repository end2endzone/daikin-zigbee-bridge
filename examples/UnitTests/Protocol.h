#pragma once

#include <stdint.h>
#include <inttypes.h>
#include "SerialInterface.h"

class Protocol
{
public:
  typedef void (*MessageCallback)(uint8_t msg_id, const uint8_t *msg_payload, uint16_t msg_payload_size);

  typedef struct message_info_s {
    uint8_t id; // unique id, indicate the type of payload, serialized
    uint8_t * payload; // payload actual bytes, serialized
    uint16_t payload_size; // size of the payload in bytes, not serialized
    uint8_t crc; // checksum for id, payload bytes, serialized
  } message_info_t;

  typedef struct message_header_s {
    uint8_t id;
    uint8_t payload; // first byte of the payload bytes
  } message_header_t;

  typedef struct message_footer_s {
    uint8_t crc;
  } message_footer_t;
 
  Protocol(SerialInterface * serial, uint8_t *buffer, uint16_t buffer_size);

  void loop();
  void sendMessage(uint8_t msg_id, const uint8_t *msg_payload, uint16_t msg_payload_size);
  void onMessageReceived(MessageCallback cb);

private:
  SerialInterface * port;

  static constexpr uint8_t SYNC_SIGNATURE_SIZE = 7;
  static constexpr uint8_t SYNC_SIGNATURE[SYNC_SIGNATURE_SIZE] = {0xAA, 0x55, 'D', 'Z', 'A', '4', 'H'};

  uint8_t sync_window[SYNC_SIGNATURE_SIZE] = {0};

  enum ParseState
  {
    WAIT_SYNC,
    WAIT_LEN1,
    WAIT_LEN2,
    WAIT_PAYLOAD
  };

  ParseState state = WAIT_SYNC;

  //message_info_t read_msg;
  uint16_t read_msg_length = 0; // a variable for storing the size of the incomming serialized fields of `message_info_t` sent through sendMessage()

  uint8_t *read_msg_buffer = nullptr;
  uint16_t read_msg_index_pos = 0;
  uint16_t read_msg_buffer_size = 0;

  MessageCallback callback = nullptr;

  bool checkSync();
  uint8_t calcCRC(const uint8_t *data, uint16_t length);
  uint8_t calcCRC(uint8_t msg_id, const uint8_t *data, uint16_t length);
};
