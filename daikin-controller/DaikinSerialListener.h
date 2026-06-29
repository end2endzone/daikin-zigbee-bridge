#pragma once

#include <Arduino.h>
#include <SerialTransfer.h>
#include "DaikinSerialApi.h"
#include "daikin_messages.h"

// ---------------------------------------------------------------------------
// DaikinSerialListener
// Polls SerialTransfer, dispatches on packetID, and delegates to DaikinSerialApi.
// ---------------------------------------------------------------------------
class DaikinSerialListener
{
private:
  SerialTransfer  _transfer;
  DaikinSerialApi *_api;

public:
  DaikinSerialListener() : _api(nullptr) {}

  void begin(HardwareSerial& port, DaikinSerialApi *api) {
    _api = api;
    _transfer.begin(port);
  }

  // Call from loop() to process any incoming packets.
  void loop() {
    if (!_transfer.available())
      return;

    switch (_transfer.currentPacketID()) {
      case PACKET_ID_GET_IP_ADDRESS_REQUEST:
        log_i("Received GET_IP_ADDRESS request.");
        _handle_get_ip_address();
        break;

      case PACKET_ID_SET_TARGET_TEMP_REQUEST:
        log_i("Received SET_TARGET_TEMP request.");
        _handle_set_target_temp();
        break;

      case PACKET_ID_GET_STATUS_REQUEST:
        log_i("Received GET_STATUS request.");
        _handle_get_status();
        break;

      default:
        log_e("Unknown packetID=%d", _transfer.currentPacketID());
        break;
    }
  }

private:
  void _handle_get_ip_address() {
    // Deserialise request (reserved byte only; kept for future extensibility)
    get_ip_address_request_t request = {};
    _transfer.rxObj(request, 0);

    // Delegate to the API implementation
    get_ip_address_response_t response = {};
    response.result = _api->getIpAddress(&response.addr);

    // Serialise and send response
    uint16_t send_size = _transfer.txObj(response, 0);
    _transfer.sendData(send_size, PACKET_ID_GET_IP_ADDRESS_RESPONSE);
  }

  void _handle_set_target_temp() {
    // Deserialise request
    set_target_temp_request_t request = {};
    _transfer.rxObj(request, 0);

    // Delegate to the API implementation
    set_target_temp_response_t response = {};
    response.result = _api->setTargetTemperature(request.temperature);

    // Serialise and send response
    uint16_t send_size = _transfer.txObj(response, 0);
    _transfer.sendData(send_size, PACKET_ID_SET_TARGET_TEMP_RESPONSE);
  }

  void _handle_get_status() {
    // Deserialise request (reserved byte only; kept for future extensibility)
    get_status_request_t request = {};
    _transfer.rxObj(request, 0);

    // Delegate to the API implementation
    get_status_response_t response = {};
    response.result = _api->getStatus(&response.status);

    // Serialise and send response
    uint16_t send_size = _transfer.txObj(response, 0);
    _transfer.sendData(send_size, PACKET_ID_GET_STATUS_RESPONSE);
  }
};
