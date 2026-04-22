#include "Protocol.h"
#include "esp32-hal-log.h"
#include <esp_log.h>

Protocol::Protocol(SerialInterface * serial, uint8_t *externalBuffer, uint16_t externalBufferSize) :
  port(serial),
  buffer(externalBuffer),
  bufferSize(externalBufferSize)
{
}

void Protocol::onMessageReceived(MessageCallback cb)
{
  callback = cb;
}

bool Protocol::checkSync()
{
  return memcmp(syncWindow, syncSignature, SYNC_LEN) == 0;
}

uint8_t Protocol::calcCRC(const uint8_t *data, uint16_t length)
{
  return calcCRC(0, data, length);
}

uint8_t Protocol::calcCRC(uint8_t msgId, const uint8_t *data, uint16_t length)
{
  uint8_t crc = msgId;
  for (uint16_t i = 0; i < length; i++)
    crc ^= data[i];
  return crc;
}

void Protocol::sendMessage(uint8_t msgId, const uint8_t *payload, uint16_t payloadLen)
{
  static const uint16_t message_id_size = sizeof(msgId);
  static const uint16_t crc_size = 1;

  uint16_t lenField = message_id_size + payloadLen + crc_size;
  uint8_t crc = calcCRC(msgId, payload, payloadLen);

  port->write(syncSignature, SYNC_LEN);
  port->write((uint8_t)(lenField >> 8));
  port->write((uint8_t)(lenField & 0xFF));
  port->write(msgId);
  port->write(payload, payloadLen);
  port->write(crc);
}

void Protocol::loop()
{
  while (port->available() > 0)
  {
    uint8_t b = port->read();

    // Shift sync window by 1 byte.
    // Use memmove() since source and destination buffers overlaps.
    memmove(syncWindow, syncWindow + 1, SYNC_LEN - 1);
    syncWindow[SYNC_LEN - 1] = b;

    if (checkSync())
    {
      if (state != WAIT_SYNC)
      {
        log_e("Unexpected sync detected. State=%d msgLength=%u indexPos=%u", state, msgLength, indexPos);
      }
      state = WAIT_LEN1;
      continue;
    }

    switch (state)
    {
    case WAIT_SYNC:
      break;

    case WAIT_LEN1:
      msgLength = ((uint16_t)b) << 8;
      state = WAIT_LEN2;
      break;

    case WAIT_LEN2:
      msgLength |= b;
      indexPos = 0;

      // There is no need to reset the machine state even if already know the size of the message is too big for our buffer.
      // Continue reading the incomming bytes to our local buffer until we fill out the buffer.
      // This simplify the error handling code since it is processed all at the same location.
      //
      //if (msgLength > bufferSize)
      //{
      //  log_e("Buffer overflow: msgLength=%u bufferSize=%u", msgLength, bufferSize);
      //  state = WAIT_SYNC;
      //  break;
      //}

      state = WAIT_PAYLOAD;
      break;

    case WAIT_PAYLOAD:
      // Check overflow
      if (indexPos >= bufferSize)
      {
        log_e("Buffer overflow during payload: msgLength=%u bufferSize=%u", msgLength, bufferSize);
        state = WAIT_SYNC;
        break;
      }

      // Append to our buffer
      buffer[indexPos] = b;
      indexPos++;

      // Message is 100% received
      if (indexPos >= msgLength)
      {
        // Check CRC
        uint8_t crc = buffer[msgLength - 1];
        uint8_t expected_crc = calcCRC(buffer, msgLength - 1);
        if (crc != expected_crc)
        {
          log_e("CRC mismatch: got=%02X expected=%02X", crc, expected_crc);
          state = WAIT_SYNC;
          break;
        }
        
        // Call the message callback function.
        if (callback)
        {
          uint8_t msgId = buffer[0];
          const uint8_t *payload = &buffer[1];
          uint16_t payloadLen = msgLength - 2;
          callback(msgId, payload, payloadLen);
        }

        state = WAIT_SYNC;
      }
      break;
    }
  }
}
