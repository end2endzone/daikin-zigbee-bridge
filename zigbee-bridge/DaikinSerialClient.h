#pragma once

#include <SerialTransfer.h>
#include "DaikinSerialApi.h"
#include "daikin_messages.h"

class DaikinSerialClient : public DaikinSerialApi
{
public:
  DaikinSerialClient();

  // Initialise the underlying SerialTransfer instance.
  // Call this from setup() after the HardwareSerial port has been started.
  void begin(HardwareSerial& port);

  // -------------------------------------------------------------------------
  // DaikinSerialApi overrides
  // -------------------------------------------------------------------------

  ApiResult getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override;

  ApiResult setTargetTemperature(int16_t temperature, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override;

  ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms = DEFAULT_TIMEOUT_MS) override;

private:
  SerialTransfer transfer;

  // Block until a packet with expected_packet_id arrives or the deadline
  // (millis() >= deadline_ms) passes.  Returns true on packet received.
  bool waitForPacket(uint8_t expected_packet_id, unsigned long deadline_ms);
};
