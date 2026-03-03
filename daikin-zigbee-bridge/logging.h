#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

const char * logBaseFileName(const char * path);
void logEntry(const char* fmt, ...);
void logError(esp_err_t err, const char * file, int line);

#define ZB_LOG_ERROR(err) logError(err, __FILE__, __LINE__);
#define CURRENT_LOG_FILE logBaseFileName(__FILE__)
#define LOG_LINE logEntry("%s(%d): [%s] Tracing line %d", __FILE__, __LINE__, __FUNCTION__, __LINE__);
