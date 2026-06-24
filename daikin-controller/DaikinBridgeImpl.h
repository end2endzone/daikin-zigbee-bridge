#pragma once

#include <Arduino.h>
#include <SerialTransfer.h>
#include "DaikinSerialApi.h"
#include "DaikinHTTP.h"

extern bool daikinPullInfo();
extern bool daikinPullAndPrintInfo();
extern void daikinPrintInfo();

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
    log_i("Requesting Daikin IP...");

    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    // Fill output object
    String local_ip = WiFi.localIP().toString();
    snprintf(addr->ip, sizeof(addr->ip), "%s", local_ip.c_str());

    log_i("Daikin IP: %s", local_ip.c_str());

    return API_RESULT_OK;
  }

  ApiResult setTargetTemperature(int16_t temperature, unsigned long timeout_ms) override
  {
    log_i("Updating Daikin target temperature to %.2f°C...", temperature / 100.0);

    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    // Implement request
    _http->setTargetTemp(temperature / 100.0);
    if (!_http->push()) {
      log_e("*** Failed to push Daikin device info.");
      return API_RESULT_DAIKIN_INFO_PUSH_FAIL;
    }

    log_i("Target temperature set to %.2f°C...", temperature / 100.0);

    return API_RESULT_OK;
  }

  static void daikinHttp2DaikinStatusInfo(DaikinHTTP * http, daikin_status_info_t *status) {
    // Fill status_info structure
    snprintf(status->name, sizeof(status->name), "%s", http->getDeviceName().c_str());
    status->power = http->getPower();
    status->mode = http->getMode();
    status->fan_rate = http->getFanRate();
    status->fan_dir = http->getFanDir();
    status->preset = http->getPreset();

    float target_temp  = http->getTargetTemp();
    float indoor_temp  = http->getIndoorTemp();
    float outdoor_temp = http->getOutdoorTemp();

    status->compressor_freq = http->getCompressorFrequency();

    status->target_temp  = (uint16_t)(target_temp  * 100);
    status->indoor_temp  = (uint16_t)(indoor_temp  * 100);
    status->outdoor_temp = (uint16_t)(outdoor_temp * 100);
  }

  ApiResult getStatus(daikin_status_info_t *status, unsigned long timeout_ms) override
  {
    log_i("Getting Daikin status...");

    _http->setTimeout(timeout_ms);
    if (!daikinPullInfo()) {
      return API_RESULT_DAIKIN_INFO_PULL_FAIL;
    }

    daikinHttp2DaikinStatusInfo(_http, status);
    
    // Print status
    daikinPrintInfo();

    return API_RESULT_OK;
  }
};
