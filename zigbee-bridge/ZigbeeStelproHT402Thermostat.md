# Zigbee Reference - zigbee-bridge

## Overview

This document describes all Zigbee clusters and attributes implemented by the `ZigbeeStelproH420Thermostat` class in the **zigbee-bridge** sketch. The class emulates a **Stelpro HT402 (Hilo)** line-voltage baseboard thermostat, allowing it to be recognised natively by Zigbee2MQTT and compatible coordinators without any custom converter.

### Device Identity

| Property | Value |
|---|---|
| Zigbee Endpoint | **25** (`0x19`) |
| Application Profile | Home Automation (`0x0104`) |
| Device Type | HA Thermostat (`ESP_ZB_HA_THERMOSTAT_DEVICE_ID`) |
| Manufacturer Name | `Stelpro` |
| Model Identifier | `HT402` |
| Manufacturer Code | `0x1185` |
| Role | End Device (Zigbee ED) |
| Node Power Source | Constant Power (mains) |
| Maximum Load | 4 000 W @ 240 V |

---

## Temperature Encoding

All temperature-related attributes in this project use the **standard ZCL encoding**: values are _signed 16-bit integers_ expressed in **hundredths of a degree Celsius (0.01 °C resolution)**. To convert a raw attribute value to a human-readable temperature, divide by `100.0`.

The ZCL specification defines `0x8000` (32768) special value meaning _Invalid_ or _Not Available_.

**Examples:**

| Raw Value | Human-Readable |
|---|---|
| `2200` | 22.0 °C |
| `2000` | 20.0 °C |
| `500` | 5.0 °C |
| `3000` | 30.0 °C |
| `-3200` | −32.0 °C |
| `0x8000` / 32768 | Invalid / not available |

---

## Clusters Summary :

The following Zigbee clusters are registered on endpoint 25.

| # | Cluster Name | Cluster ID | Role | attr_count | attr_desc_list/attr_list | role_mask | manuf_code | cluster_init |
|---|---|---|---|---|---|---|---|---|
| 0 | Basic | `0x0000` | Server | 0 | `0x4081d788` | Server | `0x0000` | `0x42018cac` |
| 1 | Identify | `0x0003` | Server | 0 | `0x4081d888` | Server | `0x0000` | `0x4201a3c0` |
| 2 | Groups | `0x0004` | Server | 0 | `0x4081da68` | Server | `0x0000` | `0x420390f6` |
| 3 | Thermostat | `0x0201` | Server | 0 | `0x4081d910` | Server | `0x0000` | `0x4201a924` |
| 4 | Thermostat UI Configuration | `0x0204` | Server | 0 | `0x4081daf0` | Server | `0x0000` | `0x420418ca` |

----

## Attributes Summary :

The following Zigbee attributes are registered on endpoint 25.

### Cluster 0 - Basic (`0x0000`)

Attributes:

| # | Attr  | Type | Size | Access | manuf_code | data_p | Value | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | Cluster Revision (`0xfffd`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4087cde0` | `4` | - | - | - |  |
| 1 | ZCL Version (`0x0000`) | Unsigned 8-bit Value (`0x0020`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081d840` | `8` | - | - | - |  |
| 2 | Power Source (`0x0007`) | 8-bit Enumeration (`0x0030`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081d874` | `0` | - | - | - |  |
| 3 | Application Version (`0x0001`) | Unsigned 8-bit Value (`0x0020`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081dc9c` | `33` | - | - | - |  |
| 4 | Date Code (`0x0006`) | Character String (`0x0042`)| 4294967294 | Read Only (`0x0001`) | `0xffff` | `0x4081dcd0` | `20000000 00000` | - | - | - |  |
| 5 | Hardware Version (`0x0003`) | Unsigned 8-bit Value (`0x0020`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081dd10` | `1` | - | - | - |  |
| 6 | Location Description (`0x0010`) | Character String (`0x0042`)| 4294967294 | Read/Write (`0x0003`) | `0xffff` | `0x4081dd44` | `Thermostat` | - | - | - |  |
| 7 | Physical Environment (`0x0011`) | 8-bit Enumeration (`0x0030`)| 1 | Read/Write (`0x0003`) | `0xffff` | `0x4081dd80` | `0` | - | - | - |  |
| 8 | Stack Version (`0x0002`) | Unsigned 8-bit Value (`0x0020`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081ddb4` | `34` | - | - | - |  |


### Cluster 1 - Identify (`0x0003`)

Attributes:

| # | Attr  | Type | Size | Access | manuf_code | data_p | Value | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | Cluster Revision (`0xfffd`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4081d8c8` | `4` | - | - | - |  |
| 1 | Identify Time (`0x0000`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read/Write (`0x0003`) | `0xffff` | `0x4081d8fc` | `0` | - | - | - | Duration in seconds the device stays in identify mode. |


### Cluster 2 - Groups (`0x0004`)

Attributes:

| # | Attr  | Type | Size | Access | manuf_code | data_p | Value | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | Cluster Revision (`0xfffd`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4081daa8` | `4` | - | - | - |  |
| 1 | Unknown Smart Cluster Attribute (`0x0000`) | 8-bit Bitmap (`0x0018`)| 1 | Read Only (`0x0001`) | `0xffff` | `0x4081dadc` | `b00000000` | - | - | - |  |


### Cluster 3 - Thermostat (`0x0201`)

Attributes:

| # | Attr  | Type | Size | Access | manuf_code | data_p | Value | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | Cluster Revision (`0xfffd`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4081d950` | `4` | - | - | - |  |
| 1 | Local Temperature (`0x0000`) | Signed 16-bit Value (`0x0029`)| 2 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081d984` | `-1` | 0.01°C | - | - | Value `0x8000` means _Not Available_. |
| 2 | Occupied Cooling Setpoint (`0x0011`) | Signed 16-bit Value (`0x0029`)| 2 | Scene, Read/Write (`0x0013`) | `0xffff` | `0x4081d9b8` | `3500` | 0.01°C | 500 | 3500 | Set artificially high so it never constrains the heating setpoint. |
| 3 | Occupied Heating Setpoint (`0x0012`) | Signed 16-bit Value (`0x0029`)| 2 | Unknown Access Type (`0x0017`) | `0xffff` | `0x4081d9ec` | `2000` | 0.01°C | 500 | 3000 | Writing a value outside the valid range will result in a INVALID_VALUE ZCL status. Must remain below Occupied Cooling Setpoint to avoid ZCL enforcement errors. |
| 4 | Control Sequence Of Operation (`0x001b`) | 8-bit Enumeration (`0x0030`)| 1 | Read/Write (`0x0003`) | `0xffff` | `0x4081da20` | `4` | - | - | - |  |
| 5 | System Mode (`0x001c`) | 8-bit Enumeration (`0x0030`)| 1 | Unknown Access Type (`0x0017`) | `0xffff` | `0x4081da54` | `4` | - | - | - | Changing this attribute also synchronises StelproSystemMode (0x401C). Both carry identical semantics and are always kept in sync: writing either one causes the other to be updated immediately. |
| 6 | Thermostat Running State (`0x0029`) | 16-bit Bitmap (`0x0019`)| 2 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081dbcc` | `b0000000000000000` | - | - | - |  |
| 7 | PI Heating Demand (`0x0008`) | Unsigned 8-bit Value (`0x0020`)| 1 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081dc00` | `0` | % | 0 | 100 | Percentage of heating demand. Must not be set to a non-zero value unless `running_state` has the `HEAT` bit set. Must be reset to `0` before clearing the `HEAT` bit. Zigbee2MQTT assumes range `[0, 255]` but ESP ZCL specification uses `[0, 100]`. Stelpro HT402 thermostat's behavior also confirms the range `[0, 100]`. |
| 8 | Outdoor Temperature (`0x0001`) | Signed 16-bit Value (`0x0029`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4081dc34` | `0` | 0.01°C | - | - | ZCL standard outdoor temperature, intended for a physical sensor on the device. |
| 9 | Occupancy (`0x0002`) | 8-bit Bitmap (`0x0018`)| 1 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081dc68` | `b00000000` | - | - | - |  |
| 10 | StelproOutdoorTemperature (`0x4001`) | Signed 16-bit Value (`0x0029`)| 2 | Reporting, Read/Write (`0x0007`) | `0xffff` | `0x4081dde8` | `0` | 0.01°C | -9900 | 19900 | Outdoor temperature displayed on the thermostat face. |
| 11 | StelproSystemMode (`0x401c`) | 8-bit Enumeration (`0x0030`)| 1 | Scene, Read/Write (`0x0013`) | `0xffff` | `0x4081de1c` | `4` | - | - | - | Mirror of the standard `SystemMode` attribute (`0x001C`). Both carry identical semantics and are always kept in sync: writing either one causes the other to be updated immediately. |
| 12 | StelproPower (`0x4008`) | Unsigned 16-bit Value (`0x0021`)| 2 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081de50` | `0` | W | 0 | 4000 | Instantaneous electrical power draw of the baseboard heater. Updated at runtime from the heating demand calculation. Zigbee2MQTT exposes this value directly in Watts. |
| 13 | StelproEnergy (`0x4009`) | Unsigned 32-bit Value (`0x0023`)| 4 | Reporting, Read Only (`0x0005`) | `0xffff` | `0x4081de84` | `0` | Wh | - | - | Cumulative energy consumption since last reset. The on-wire value is in **Watt-hours (Wh)**. Zigbee2MQTT divides by `1000` before publishing, so Home Assistant receives the value in **kWh**. |
| 14 | StelproPeakDemandIcon (`0x4105`) | Unsigned 16-bit Value (`0x0021`)| 2 | Reporting, Read/Write (`0x0007`) | `0xffff` | `0x4081deb8` | `0` |  | 0 | 64800 | Set peak demand event icon for the specified number of seconds. |


### Cluster 4 - Thermostat UI Configuration (`0x0204`)

Attributes:

| # | Attr  | Type | Size | Access | manuf_code | data_p | Value | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | Cluster Revision (`0xfffd`) | Unsigned 16-bit Value (`0x0021`)| 2 | Read Only (`0x0001`) | `0xffff` | `0x4081db30` | `4` | - | - | - |  |
| 1 | Temperature Display Mode (`0x0000`) | 8-bit Enumeration (`0x0030`)| 1 | Reporting, Read/Write (`0x0007`) | `0xffff` | `0x4081db64` | `0` | - | - | - |  |
| 2 | Keypad Lockout (`0x0001`) | 8-bit Enumeration (`0x0030`)| 1 | Reporting, Read/Write (`0x0007`) | `0xffff` | `0x4081db98` | `0` | - | - | - |  |

---

## Key Implementation Notes

### Cooling Setpoint as a Constraint Guard

The `OccupiedCoolingSetpoint` is intentionally initialised to `3500` (35.0 °C) - well above the maximum heating setpoint of `3000` (30.0 °C). The ZCL stack enforces that `occupied_heating_setpoint` must remain strictly less than `occupied_cooling_setpoint`. If the cooling setpoint were left at the SDK default of 26.0 °C, any attempt to set the heating setpoint above 25.0 °C would fail with a ZCL `INVALID_VALUE` status.

### _System Mode_ / _Stelpro's System Mode_ Synchronisation

* Both `SystemMode` (0x001C) and `StelproSystemMode` (0x4001) attributes are declared as 8bit enum inside cluster 0x0201. They carry the same data type.
* Multiple other sources maps `StelproSystemMode` (0x4001) to `SystemMode` (0x001C) for their implementation. This is convincing arguments that both attributes carry identical semantic.
* To make sure they are always synchronized, both attributes should be configured so they effectively share the same data pointer.
  * This would garanty that any read or writes of either attribute always affect the live hardware state, regardless of which attribute was most recently written by the coordinator. Both attribute reflects identical state in firmware.
  * Calling function `esp_zb_cluster_add_attr()` with `value_p` with the same address as `system_mode->data_p` is not possible.
  * The SDK's implementation is assumed to make a copy of the given `value_p` memory address to a new allocation space. This is why creating a new attribute with the same memory address result in the following:
    ```
    Found existing 'system_mode' attribute: id=0x001c (System Mode), type=0x30 (8-bit Enumeration), access=0x13 (Scene, Read/Write), manuf_code=0xffff, data_p=0x4081cee4, data=4
    Create new 'stelpro_system_mode' attribute: id=0x401c (Unknown Thermostat Cluster Attribute), type=0x30 (8-bit Enumeration), access=0x13 (Scene, Read/Write), manuf_code=0xffff, data_p=0x4081d300, data=4
    ```
  * In other words, notice `system_mode->data_p=0x4081cee4` and calling `esp_zb_cluster_add_attr(with value_p=0x4081cee4)` actually create an attribute with `data_p=0x4081d300` which is a different memory address.
  * A potential option is to redirect data_p to share system_mode's backing storage after its creation. However, this creates an unstable application. The SDK do not like/acept to have 2 attributes pointing to the same data pointer. Doing so result in a crash at runtime:
    ```
    Found `system_mode` attribute: id=0x001c (System Mode), type=0x30 (8-bit Enumeration),     access=0x13 (Scene, Read/Write), manuf_code=0xffff, data_p=0x4081cee4, data=4
    Created `stelpro_system_mode` attribute: id=0x401c (Unknown Thermostat Cluster Attribute),     type=0x30 (8-bit Enumeration), access=0x13 (Scene, Read/Write), manuf_code=0xffff,     data_p=0x4081d300, data=4
    Updated `stelpro_system_mode` attribute: id=0x401c (Unknown Thermostat Cluster Attribute),     type=0x30 (8-bit Enumeration), access=0x13 (Scene, Read/Write), manuf_code=0xffff,     data_p=0x4081cee4, data=4
    [...]
    Starting Zigbee stack...
    CORRUPT HEAP: Bad head at 0x4081cedc. Expected 0xabba1234 got 0x40818fc4
    assert failed: multi_heap_free multi_heap_poisoning.c:279 (head != NULL)
    ```

I summary, a shared `data_p` pointer would be the natural implementation, but redirecting the ESP Zigbee SDK's internal heap pointer after attribute creation results in heap corruption (`CORRUPT HEAP: Bad head`) at runtime.

Conclusion: synchronisation is therefore performed explicitly in `zbAttributeSet()` and `setStelproSystemMode()` via the helper method `updateSystemModes()`: whenever either attribute is written, the other is immediately updated to the same value.

### Energy Accumulation

Energy (`0x4009`) is accumulated over time using the formula:

```
ΔEnergy (Wh) = Power (W) × ΔTime (hours)
```

The update interval is defined by `SIMULATION_UPDATE_INTERVAL` (default 5 000 ms). The value accumulates monotonically and is never automatically reset; resetting requires an explicit call to `setStelproEnergy(0)`.

### PI Heating Demand 

**Range of values**:

* The attribute represent a _Percentage of Heating Demand_. As a percentage, it must be in range [0, 100]. See macros `ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE` & `ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE` which defines the minimum and maximum values.
* _Zigbee2mqtt_ assumes values are in [0,255] range. This is likely a bug or an incomplete implementation.
* Other zigbee projects (including esp-zigbee-sdk library) also assume values in [0,100] range.
* Also confirmed with a real Stelpro HT402 thermostat, pi_heating_demand must be in range [0, 100].


**Ordering Constraint**:

The `PIHeatingDemand` attribute (`0x0008`) has a strict ordering requirement enforced by the ZCL stack:

- The HEAT bit in `RunningState` (`0x0029`) **must be set before** `PIHeatingDemand` is set to a non-zero value.
- `PIHeatingDemand` **must be reset to `0` before** the HEAT bit is cleared from `RunningState`.

Violating this order results in a ZCL error. The simulation loop in `zigbee-bridge.ino` accounts for this ordering explicitly.

### StelproOutdoorTemp Registration Method

Custom attributes added with `esp_zb_cluster_add_manufacturer_attr()` using a non-`0xFFFF` manufacturer code are rejected by Zigbee2MQTT with `UNSUPPORTED_ATTRIBUTE`. `StelproOutdoorTemp` (`0x4001`) is therefore registered using the plain `esp_zb_cluster_add_attr()` function, which registers it without a manufacturer-specific code and makes it universally accessible.

### Thermostat UI Config, Keypad Lockout

* Can be set by the device itself but the new value won't be reflected in zigbee2mqtt. zigbee2mqtt might simple not watching the attribute for changes.
* TODO: validate with a real Stelpro thermostat.
