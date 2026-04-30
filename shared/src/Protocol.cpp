#include "Protocol.h"
#include "esp32-hal-log.h"
#include <esp_log.h>
#include <SoftTimers.h>

Protocol::Protocol(SerialInterface * serial, uint8_t *buffer, uint16_t buffer_size) :
  port(serial),
  state(WAIT_SYNC),
  read_msg_length(0),
  read_msg_buffer(buffer),
  read_msg_buffer_size(buffer_size),
  read_msg_index_pos(0),
  callback(nullptr)
{
}

void Protocol::onMessageReceived(MessageCallback cb)
{
  callback = cb;
}

bool Protocol::checkSync()
{
  return memcmp(sync_window, SYNC_SIGNATURE, SYNC_SIGNATURE_SIZE) == 0;
}

uint8_t Protocol::calcCRC(const uint8_t *data, uint16_t length)
{
  return calcCRC(0, data, length);
}

uint8_t Protocol::calcCRC(uint8_t msg_id, const uint8_t *data, uint16_t length)
{
  uint8_t crc = msg_id; // CRC is initialized to 0, but since we also include msg_id in the checksum, we initialize to `msg_id` because `0 ^= msg_id` equals `msg_id`.
  for (uint16_t i = 0; i < length; i++)
    crc ^= data[i];
  return crc;
}

size_t Protocol::sendMessage(uint8_t msg_id, const uint8_t *msg_payload, uint16_t msg_payload_size)
{
  static const uint16_t message_id_size = sizeof(msg_id);
  static const uint16_t crc_size = 1;

  uint16_t write_msg_length = message_id_size + msg_payload_size + crc_size;
  uint8_t crc = calcCRC(msg_id, msg_payload, msg_payload_size);

  size_t write_total = 0;
  size_t last_write_size =0;

  #define ASSERT_WRITE_OK(expression) { \
    last_write_size = (expression); \
    if (last_write_size == 0) return 0; \
    write_total += last_write_size; \
  }

  // Write the SYNC_SIGNATURE bytes to trigger the beginnig of a new message
  ASSERT_WRITE_OK(port->write(SYNC_SIGNATURE, SYNC_SIGNATURE_SIZE));

  // Write the size of the message_info_t serialized fields
  ASSERT_WRITE_OK(port->write((uint8_t)(write_msg_length >> 8)));
  ASSERT_WRITE_OK(port->write((uint8_t)(write_msg_length & 0xFF)));

  // Write message_info_t serialized field bytes
  ASSERT_WRITE_OK(port->write(msg_id));
  if (msg_payload_size > 0) {
    ASSERT_WRITE_OK(port->write(msg_payload, msg_payload_size));
  }
  ASSERT_WRITE_OK(port->write(crc));

  #undef ASSERT_WRITE_OK
  
  return write_total;
}

void Protocol::loop()
{
  while (port->available() > 0)
  {
    uint8_t b = port->read();

    // Shift sync window by 1 byte.
    // Use memmove() since source and destination buffers overlaps.
    memmove(sync_window, sync_window + 1, SYNC_SIGNATURE_SIZE - 1);
    sync_window[SYNC_SIGNATURE_SIZE - 1] = b;

    // A re-sync can be sent at any time
    if (checkSync())
    {
      if (state != WAIT_SYNC)
      {
        log_e("Unexpected sync detected. State=%d read_msg_length=%u read_msg_index_pos=%u", state, read_msg_length, read_msg_index_pos);
      }
      state = WAIT_LEN1;
      read_msg_length = 0;
      continue;
    }

    switch (state)
    {
    case WAIT_SYNC:
      break;

    case WAIT_LEN1:
      read_msg_length = ((uint16_t)b) << 8;
      state = WAIT_LEN2;
      break;

    case WAIT_LEN2:
      read_msg_length |= b;
      read_msg_index_pos = 0;

      // There is no need to reset the machine state even if already know the size of the message is too big for our msg_buffer.
      // Continue reading the incomming bytes to our local msg_buffer until we fill out the msg_buffer.
      // This simplify the error handling code since it is processed all at the same location.
      //
      //if (read_msg_length > msg_buffer_size)
      //{
      //  log_e("Buffer overflow: read_msg_length=%u msg_buffer_size=%u", read_msg_length, msg_buffer_size);
      //  state = WAIT_SYNC;
      //  break;
      //}

      state = WAIT_PAYLOAD;
      break;

    case WAIT_PAYLOAD:
      // Check overflow
      if (read_msg_index_pos >= read_msg_buffer_size)
      {
        log_e("Buffer overflow during msg_payload: read_msg_length=%u read_msg_buffer_size=%u", read_msg_length, read_msg_buffer_size);
        state = WAIT_SYNC;
        read_msg_index_pos = 0;
        break;
      }

      // Append to our msg_buffer
      read_msg_buffer[read_msg_index_pos] = b;
      read_msg_index_pos++;

      // Message is 100% received
      if (read_msg_index_pos >= read_msg_length)
      {
        // Define message header and footer based on assumed transmissed message size `read_msg_length`.
        message_header_t * msg_header = (message_header_t *)&read_msg_buffer[0];
        message_footer_t * msg_footer = (message_footer_t *)&read_msg_buffer[read_msg_length - sizeof(message_footer_t)];
        uint16_t msg_payload_size = read_msg_length - sizeof(message_header_t)+1 - sizeof(message_footer_t);

        // Check CRC
        const uint8_t & expected_crc = msg_footer->crc;
        const uint8_t calculated_crc = calcCRC(msg_header->id, &msg_header->payload, msg_payload_size);
        if (calculated_crc != expected_crc)
        {
          log_e("CRC mismatch: calculated=%02X expected=%02X", calculated_crc, expected_crc);
          state = WAIT_SYNC;
          read_msg_length = 0;
          break;
        }
        
        // Call the message callback function.
        if (callback)
        {
          callback(msg_header->id, &msg_header->payload, msg_payload_size);
        }

        state = WAIT_SYNC;
        read_msg_length = 0;
      }
      break;
    }
  }
}

size_t Protocol::flushReadBuffer(unsigned long timeout_time) {
  size_t read_size = 0;

  // flush all existing bytes from the serial's input buffer
  while (port->available() > 0)
  {
    port->read();
    read_size++;
  }

  // keep flushing for a specific amount of time
  if (timeout_time > 0) {
    SoftTimer timer;
    timer.setTimeOutTime(timeout_time);
    timer.reset();
    while(!timer.hasTimedOut()) {
      while (port->available() > 0)
      {
        port->read();
        read_size++;
      }
    }
  }

  return read_size;
}
