// DaikinSerialClient.cpp
//
// SerialTransfer-based implementation of DaikinSerialApi.

#include "DaikinSerialClient.h"
#include <Arduino.h>    // millis()

DaikinSerialClient::DaikinSerialClient() {
}

void DaikinSerialClient::begin(HardwareSerial& port) {
  transfer.begin(port);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool DaikinSerialClient::waitForPacket(uint8_t expected_packet_id, unsigned long deadline_ms) {
  while (millis() < deadline_ms) {
    if (transfer.available()) {
      if (transfer.currentPacketID() == expected_packet_id)
        return true;
      // A packet arrived but its ID does not match; discard and keep polling.
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// getIpAddress
// ---------------------------------------------------------------------------

DaikinSerialApi::ApiResult DaikinSerialClient::getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms) {
  // Build and send request
  get_ip_address_request_t request = {};
  request.reserved = 0x00;
  uint16_t send_size = transfer.txObj(request, 0);
  transfer.sendData(send_size, PACKET_ID_GET_IP_ADDRESS_REQUEST);

  // Wait for matching response
  unsigned long deadline_ms = millis() + timeout_ms;
  if (!waitForPacket(PACKET_ID_GET_IP_ADDRESS_RESPONSE, deadline_ms))
    return API_RESULT_TIMEOUT;

  // Deserialise response
  get_ip_address_response_t response = {};
  transfer.rxObj(response, 0);

  if (response.result == API_RESULT_OK && addr != nullptr)
    *addr = response.addr;

  return response.result;
}

DaikinSerialApi::ApiResult DaikinSerialClient::setTargetTemperature(int16_t temperature, unsigned long timeout_ms) {
  // Build and send request
  set_target_temp_request_t request = {};
  request.temperature = temperature;
  uint16_t send_size = transfer.txObj(request, 0);
  transfer.sendData(send_size, PACKET_ID_SET_TARGET_TEMP_REQUEST);

  // Wait for matching response
  unsigned long deadline_ms = millis() + timeout_ms;
  if (!waitForPacket(PACKET_ID_SET_TARGET_TEMP_RESPONSE, deadline_ms))
    return API_RESULT_TIMEOUT;

  // Deserialise response
  set_target_temp_response_t response = {};
  transfer.rxObj(response, 0);

  return response.result;
}

DaikinSerialApi::ApiResult DaikinSerialClient::getStatus(daikin_status_info_t *status, unsigned long timeout_ms) {
  // Build and send request
  get_status_request_t request = {};
  request.reserved = 0x00;
  uint16_t send_size = transfer.txObj(request, 0);
  transfer.sendData(send_size, PACKET_ID_GET_STATUS_REQUEST);

  // Wait for matching response
  unsigned long deadline_ms = millis() + timeout_ms;
  if (!waitForPacket(PACKET_ID_GET_STATUS_RESPONSE, deadline_ms))
    return API_RESULT_TIMEOUT;

  // Deserialise response
  get_status_response_t response = {};
  transfer.rxObj(response, 0);

  if (response.result == API_RESULT_OK && status != nullptr)
    *status = response.status;

  return response.result;
}
