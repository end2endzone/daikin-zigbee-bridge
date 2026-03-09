#pragma once

#include "Zigbee.h"
#include "esp_zigbee_type.h"
#include <stdint.h>

// Stelpro HT402 specific configuration
// Notes:
//   HT402 uses endpoint 25, not 10!
#define STELPRO_ENDPOINT 25
#define STELPRO_MANUFACTURER_CODE 0x1185
#define STELPRO_MANUFACTURER_NAME "Stelpro"
#define STELPRO_MODEL_NAME "HT402"

// Default Stelpro thermostat values (in hundredths of degrees Celsius)
#define STELPRO_DEFAULT_TEMPERATURE           2200  // 22.0°C
#define STELPRO_DEFAULT_HEATING_SETPOINT      2000  // 20.0°C
#define STELPRO_MIN_HEAT_SETPOINT              500  //  5.0°C
#define STELPRO_MAX_HEAT_SETPOINT             3000  // 30.0°C
#define STELPRO_TEMP_MEASUREMENT_TOLERANCE      50  //  0.5°C
// To allow `occupied_cooling_setpoint` with values from 5.0°C to 30.0°C, 
// we need to set the following attribute values:
#define STELPRO_MIN_COOL_SETPOINT              500  //  5.0°C
#define STELPRO_MAX_COOL_SETPOINT             (500 + STELPRO_MAX_HEAT_SETPOINT) // make sure value is greater than STELPRO_MAX_HEAT_SETPOINT
#define STELPRO_OCCUPIED_COOLING_SETPOINT     (500 + STELPRO_MAX_HEAT_SETPOINT) // make sure value is greater than STELPRO_MAX_HEAT_SETPOINT

// Running state values (each bit is the state of something)
#define THERMOSTAT_RUNNING_STATE_IDLE 0x0000
#define THERMOSTAT_RUNNING_STATE_HEAT (THERMOSTAT_RUNNING_STATE_IDLE | ESP_ZB_ZCL_THERMOSTAT_RUNNING_STATE_HEAT_STATE_ON_BIT)

// Stelpro outdoor temperature:
//  The displayed outdoor temperature is exposed as a custom attribute.
//  The code in https://github.com/Koenkk/zigbee-herdsman-converters, to refer to
//  strings such as `stelpro_thermostat_outdoor_temperature` and `StelproOutdoorTemp`.
//  The Zigbee specification has already a known attribute for outdoor temperature in the specification according to:
//  https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32/api-reference/zcl/esp_zigbee_zcl_thermostat.html
//  Attribute `ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID` is not related to this attribute. It refers to the actual measured temperature
//  by a physical sensor on the device and not an attribute whose purpose is to "display a value on the thermostat".
//  Setting this attribute to value 12 using zigbee2mqtt outputs the following error:
//    ```
//    [2/25/2026, 10:40:20 PM] z2m: Publish 'set' 'write' to '0x232cabcdef123456' failed: 'Error: ZCL command 0x232cabcdef123456/25 hvacThermostat.write({"StelproOutdoorTemp":12},
//    {"timeout":10000,"disableResponse":false,"disableRecovery":false,"disableDefaultResponse":true,"direction":0,"reservedBits":0,"writeUndiv":false}) failed
//    (Delivery failed for '14298'.)'
//    ```
//    where 14298 is 0x37DA.
//  Custom attributes should be greater than 0x4000.
//  The `StelproOutdoorTemp` attribute definition is available at 
//    * https://github.com/Koenkk/zigbee-specification/blob/3c96049cd632d0780b9f79ebc8aecc672d4c4505/src/zcl/definition/clusters.ts#L1906
//    * https://github.com/Koenkk/zigbee-herdsman/blob/v0.33.6/src/zcl/definition/cluster.ts#L2022
//  It is defined as `StelproOutdoorTemp: {ID: 0x4001, type: DataType.INT16}`.
#define ZB_STELPRO_ATTR_OUTDOOR_TEMP_ID 0x4001
#define ZB_STELPRO_ATTR_OUTDOOR_TEMP_MIN_VALUE  -3200
#define ZB_STELPRO_ATTR_OUTDOOR_TEMP_MAX_VALUE  19900

// Power & Energy:
//    According to https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/devices/stelpro.ts, lines 10 to 29,
//    Stelpro seems to expose power and energy in the Thermostat cluster using custom attributes:
//    * Power  as custom attribute (16392, 0x4008), as uint16_t, in Watts (W), with access Read|Reporting
//    * Energy as custom attribute (16393, 0x4009), as uint32_t, in Watt‑hour (Wh), with access Read|Reporting
#define ZB_STELPRO_ATTR_POWER_ID 0x4008
#define ZB_STELPRO_ATTR_ENERGY_ID 0x4009

// Stelpro system mode:
//  This is Stelpro attribute exposed as a custom attribute.
//  The `StelproSystemMode` attribute definition is available at
//    * https://github.com/Koenkk/zigbee-specification/blob/3c96049cd632d0780b9f79ebc8aecc672d4c4505/src/zcl/definition/clusters.ts#L1905
//    * https://github.com/Koenkk/zigbee-herdsman/blob/v0.33.6/src/zcl/definition/cluster.ts#L2021
//  It is defined as `StelproSystemMode: {ID: 0x401c, type: DataType.ENUM8}`.
#define ZB_STELPRO_ATTR_SYSTEM_MODE_ID 0x401c

// Maximum power for baseboard heater
#define STELPRO_MAX_POWER_WATTS 4000
