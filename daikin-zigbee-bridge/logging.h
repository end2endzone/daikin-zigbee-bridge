#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "esp_zigbee_type.h"

void logEntry(const char* fmt, ...);
void logError(esp_err_t err, const char * file, int line);
