#pragma once

#include <Arduino.h>
#include <SerialTransfer.h>
#include "DaikinSerialApi.h"
#include "DaikinHTTP.h"

extern bool daikinPullInfo();

class DaikinBridgeImpl : public DaikinSerialApi
{
private:
  DaikinHTTP * _http;

public:

  void begin(DaikinHTTP * http)
  {
    _http = http;
  }

  ApiResult getIpAddress(daikin_ip_address_t *addr, unsigned long timeout_ms) override
  {
    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    // Fill output object
    snprintf(addr->ip, sizeof(addr->ip), "%s", WiFi.localIP().toString().c_str());

    return API_RESULT_OK;
  }

  ApiResult setTargetTemperature(uint16_t temperature, unsigned long timeout_ms) override
  {
    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    // Implement request
    _http->setTargetTemp(temperature / 100.0);
    log_i("Updating target temperature to %.2f°C");
    if (!_http->push()) {
      log_e("*** Failed to push Daikin device info.");
      return API_RESULT_DAIKIN_INFO_PUSH_FAIL;
    }

    return API_RESULT_OK;
  }

  ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms) override
  {
    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    // Fill output object
    snprintf(status->name, sizeof(status->name), "%s", _http->getDeviceName().c_str());
    status->power = (int)_http->getPower();
    status->mode = (int)_http->getMode();
    status->fan_rate = (int)_http->getFanRate();
    status->fan_dir = (int)_http->getFanDir();
    status->preset = (int)_http->getPreset();

    float target_temp  = _http->getTargetTemp();
    float indoor_temp  = _http->getIndoorTemp();
    float outdoor_temp = _http->getOutdoorTemp();

    status->target_temp  = (uint16_t)(target_temp  * 100);
    status->indoor_temp  = (uint16_t)(indoor_temp  * 100);
    status->outdoor_temp = (uint16_t)(outdoor_temp * 100);

    return API_RESULT_OK;
  }
};
