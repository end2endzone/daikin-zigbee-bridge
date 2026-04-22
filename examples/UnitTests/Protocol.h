#pragma once

#include <stdint.h>
#include <inttypes.h>
#include "SerialInterface.h"

class Protocol
{
public:
  typedef void (*MessageCallback)(uint8_t msgId, const uint8_t *payload, uint16_t payloadLen);

  Protocol(SerialInterface * serial, uint8_t *externalBuffer, uint16_t externalBufferSize);

  void loop();
  void sendMessage(uint8_t msgId, const uint8_t *payload, uint16_t payloadLen);
  void onMessageReceived(MessageCallback cb);

private:
  SerialInterface * port;

  static constexpr uint8_t SYNC_LEN = 7;
  static constexpr uint8_t syncSignature[SYNC_LEN] = {0xAA, 0x55, 'D', 'Z', 'A', '4', 'H'};

  uint8_t syncWindow[SYNC_LEN] = {0};

  enum ParseState
  {
    WAIT_SYNC,
    WAIT_LEN1,
    WAIT_LEN2,
    WAIT_PAYLOAD
  };

  ParseState state = WAIT_SYNC;

  uint16_t msgLength = 0;
  uint16_t indexPos = 0;

  uint8_t *buffer;
  uint16_t bufferSize;

  MessageCallback callback = nullptr;

  bool checkSync();
  uint8_t calcCRC(const uint8_t *data, uint16_t length);
  uint8_t calcCRC(uint8_t msgId, const uint8_t *data, uint16_t length);
};
