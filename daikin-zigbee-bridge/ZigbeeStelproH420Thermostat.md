# Zigbee Reference - daikin-zigbee-bridge

## Overview

This document describes all Zigbee clusters and attributes implemented by the `ZigbeeStelproH420Thermostat` class in the **daikin-zigbee-bridge** project. The class emulates a **Stelpro HT402 (Hilo)** line-voltage baseboard thermostat, allowing it to be recognised natively by Zigbee2MQTT and compatible coordinators without any custom converter.

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

| # | Cluster Name | Cluster ID | Role | attr_count | attr_desc_list/attr_list | role_mask | manuf_code | cluster_init
|---|---|---|---|---|---|---|---|---|
| 0 | Basic | `0x00` | Server | 0 | `0x4081cc50` | Server | `0x0000` | `0x42010846` |
| 1 | Identify | `0x03` | Server | 0 | `0x4081cce4` | Server | `0x0000` | `0x42011f5a` |
| 2 | Groups | `0x04` | Server | 0 | `0x4081cf58` | Server | `0x0000` | `0x42030c82` |
| 3 | Thermostat | `0x201` | Server | 0 | `0x4081ce00` | Server | `0x0000` | `0x420124be` |
| 4 | Thermostat UI Configuration | `0x204` | Server | 0 | `0x4081cfe0` | Server | `0x0000` | `0x42039456` |

----

## Attributes Summary :

The following Zigbee attributes are registered on endpoint 25.

Attributes summary:
| Cluster Info | # | Attr Name | Attr ID | Type Name | Type ID | Type Size | Access Name | Access ID | manuf_code | data_p | data |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Cluster #0 Basic (`0x00`) | 0 | Unknown Basic Cluster Attribute | `0xfffd` | Unsigned 16-bit Value | `0x0021`| 2 | Read Only | `0x0001` | `0xffff` | `0x4087cde0` | `4` |
| Cluster #0 Basic (`0x00`) | 1 | ZCL Version | `0x0000` | Unsigned 8-bit Value | `0x0020`| 1 | Read Only | `0x0001` | `0xffff` | `0x4081c3d8` | `8` |
| Cluster #0 Basic (`0x00`) | 2 | Power Source | `0x0007` | 8-bit Enumeration | `0x0030`| 1 | Read Only | `0x0001` | `0xffff` | `0x4081ccd0` | `1` |
| Cluster #1 Identify (`0x03`) | 0 | Unknown Identify Cluster Attribute | `0xfffd` | Unsigned 16-bit Value | `0x0021`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081cdb8` | `4` |
| Cluster #1 Identify (`0x03`) | 1 | Identify Time | `0x0000` | Unsigned 16-bit Value | `0x0021`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081cdec` | `0` |
| Cluster #2 Groups (`0x04`) | 0 | Unknown Smart Cluster Attribute | `0xfffd` | Unsigned 16-bit Value | `0x0021`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081cf98` | `4` |
| Cluster #2 Groups (`0x04`) | 1 | Unknown Smart Cluster Attribute | `0x0000` | 8-bit Bitmap | `0x0018`| 1 | Read Only | `0x0001` | `0xffff` | `0x4081cfcc` | `b00000000` |
| Cluster #3 Thermostat (`0x201`) | 0 | Unknown Thermostat Cluster Attribute | `0xfffd` | Unsigned 16-bit Value | `0x0021`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081ce40` | `4` |
| Cluster #3 Thermostat (`0x201`) | 1 | Local Temperature | `0x0000` | Signed 16-bit Value | `0x0029`| 2 | Reporting, Read Only | `0x0005` | `0xffff` | `0x4081ce74` | `-1` |
| Cluster #3 Thermostat (`0x201`) | 2 | Occupied Cooling Setpoint | `0x0011` | Signed 16-bit Value | `0x0029`| 2 | Scene, Read/Write | `0x0013` | `0xffff` | `0x4081cea8` | `3500` |
| Cluster #3 Thermostat (`0x201`) | 3 | Occupied Heating Setpoint | `0x0012` | Signed 16-bit Value | `0x0029`| 2 | Scene, Read/Write | `0x0013` | `0xffff` | `0x4081cedc` | `2000` |
| Cluster #3 Thermostat (`0x201`) | 4 | Control Sequence Of Operation | `0x001b` | 8-bit Enumeration | `0x0030`| 1 | Read/Write | `0x0003` | `0xffff` | `0x4081cf10` | `4` |
| Cluster #3 Thermostat (`0x201`) | 5 | System Mode | `0x001c` | 8-bit Enumeration | `0x0030`| 1 | Scene, Read/Write | `0x0013` | `0xffff` | `0x4081cf44` | `4` |
| Cluster #3 Thermostat (`0x201`) | 6 | Thermostat Running State | `0x0029` | 16-bit Bitmap | `0x0019`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d0bc` | `b0000000000000000` |
| Cluster #3 Thermostat (`0x201`) | 7 | PI Heating Demand | `0x0008` | Unsigned 8-bit Value | `0x0020`| 1 | Reporting, Read Only | `0x0005` | `0xffff` | `0x4081d0f0` | `0` |
| Cluster #3 Thermostat (`0x201`) | 8 | Outdoor Temperature | `0x0001` | Signed 16-bit Value | `0x0029`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d124` | `0` |
| Cluster #3 Thermostat (`0x201`) | 9 | Occupancy | `0x0002` | 8-bit Bitmap | `0x0018`| 1 | Read Only | `0x0001` | `0xffff` | `0x4081d158` | `b00000001` |
| Cluster #3 Thermostat (`0x201`) | 10 | Abs Min Heat Setpoint Limit | `0x0003` | Signed 16-bit Value | `0x0029`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d18c` | `500` |
| Cluster #3 Thermostat (`0x201`) | 11 | Abs Max Heat Setpoint Limit | `0x0004` | Signed 16-bit Value | `0x0029`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d1c0` | `3000` |
| Cluster #3 Thermostat (`0x201`) | 12 | Min Heat Setpoint Limit | `0x0015` | Signed 16-bit Value | `0x0029`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081d1f4` | `500` |
| Cluster #3 Thermostat (`0x201`) | 13 | Max Heat Setpoint Limit | `0x0016` | Signed 16-bit Value | `0x0029`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081d228` | `3000` |
| Cluster #3 Thermostat (`0x201`) | 14 | Abs Min Cool Setpoint Limit | `0x0005` | Signed 16-bit Value | `0x0029`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d25c` | `500` |
| Cluster #3 Thermostat (`0x201`) | 15 | Abs Max Cool Setpoint Limit | `0x0006` | Signed 16-bit Value | `0x0029`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d290` | `3500` |
| Cluster #3 Thermostat (`0x201`) | 16 | Min Cool Setpoint Limit | `0x0017` | Signed 16-bit Value | `0x0029`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081d2c4` | `500` |
| Cluster #3 Thermostat (`0x201`) | 17 | Max Cool Setpoint Limit | `0x0018` | Signed 16-bit Value | `0x0029`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081d2f8` | `3500` |
| Cluster #3 Thermostat (`0x201`) | 18 | Unknown Thermostat Cluster Attribute | `0x4001` | Signed 16-bit Value | `0x0029`| 2 | Read/Write | `0x0003` | `0xffff` | `0x4081d32c` | `0` |
| Cluster #3 Thermostat (`0x201`) | 19 | Unknown Thermostat Cluster Attribute | `0x401c` | 8-bit Enumeration | `0x0030`| 1 | Scene, Read/Write | `0x0013` | `0xffff` | `0x4081d360` | `4` |
| Cluster #3 Thermostat (`0x201`) | 20 | Unknown Thermostat Cluster Attribute | `0x4008` | Unsigned 16-bit Value | `0x0021`| 2 | Reporting, Read Only | `0x0005` | `0xffff` | `0x4081d394` | `0` |
| Cluster #3 Thermostat (`0x201`) | 21 | Unknown Thermostat Cluster Attribute | `0x4009` | Unsigned 32-bit Value | `0x0023`| 4 | Reporting, Read Only | `0x0005` | `0xffff` | `0x4081d3c8` | `0` |
| Cluster #4 Thermostat UI Configuration (`0x204`) | 0 | Unknown Thermostat UI Cluster Attribute | `0xfffd` | Unsigned 16-bit Value | `0x0021`| 2 | Read Only | `0x0001` | `0xffff` | `0x4081d020` | `4` |
| Cluster #4 Thermostat UI Configuration (`0x204`) | 1 | Temperature Display Mode | `0x0000` | 8-bit Enumeration | `0x0030`| 1 | Read/Write | `0x0003` | `0xffff` | `0x4081d054` | `0` |
| Cluster #4 Thermostat UI Configuration (`0x204`) | 2 | Keypad Lockout | `0x0001` | 8-bit Enumeration | `0x0030`| 1 | Read/Write | `0x0003` | `0xffff` | `0x4081d088` | `0` |

---

## Cluster Details


### Cluster 0 - Basic (`0x0000`)

Provides static device identity and power-source information. Read-only from the coordinator's perspective.

#### Attributes

| Attr ID | Name | ZCL Type | C Type | Access | Default | Notes |
|---|---|---|---|---|---|---|
| `0xFFFD` | Cluster Revision | `uint16` | `uint16_t` | Read Only | `4` | Auto-added by SDK |
| `0x0000` | ZCL Version | `uint8` | `uint8_t` | Read Only | `8` | ZCL r8 |
| `0x0007` | Power Source | `enum8` | `uint8_t` | Read Only | `0x01` | Constant (mains) power |

---

### Cluster 1 - Identify (`0x0003`)

Allows the coordinator to trigger a visual or audible identify action on the device.

#### Attributes

| Attr ID | Name | ZCL Type | C Type | Access | Default | Notes |
|---|---|---|---|---|---|---|
| `0xFFFD` | Cluster Revision | `uint16` | `uint16_t` | Read Only | `4` | Auto-added by SDK |
| `0x0000` | Identify Time | `uint16` | `uint16_t` | Read / Write | `0` | Duration in seconds the device stays in identify mode. `0` = not identifying. |

---

### Cluster 2 - Groups (`0x0004`)

Provides group membership management. Implemented to satisfy coordinator expectations for a thermostat device profile.

#### Attributes

| Attr ID | Name | ZCL Type | C Type | Access | Default | Notes |
|---|---|---|---|---|---|---|
| `0xFFFD` | Cluster Revision | `uint16` | `uint16_t` | Read Only | `4` | Auto-added by SDK |
| `0x0000` | Name Support | `bitmap8` | `uint8_t` | Read Only | `0x00` | Bit 7: group names supported |

---

### Cluster 3 - Thermostat (`0x0201`)

The primary operational cluster. Contains all heating-control attributes, running-state indicators, setpoint limits, and all Stelpro manufacturer-specific custom attributes. Attributes in this cluster are divided into four sub-groups below.

#### 3.1 - Mandatory Standard Attributes

These five attributes are created automatically by `esp_zb_thermostat_cluster_create()`.

| Attr ID | Name | ZCL Type | C Type | Access | Default | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|
| `0x0000` | Local Temperature | `int16s` | `int16_t` | Reporting, Read Only | `0x8000` | 0.01 °C | - | - | Current measured room temperature. `0x8000` means "not available". Updated by the application at runtime. |
| `0x0011` | Occupied Cooling Setpoint | `int16s` | `int16_t` | Scene, Read / Write | `3500` (35.0 °C) | 0.01 °C | `500` | `3500` | Set artificially high so it never constrains the heating setpoint. The device is heating-only; this value is a placeholder. |
| `0x0012` | Occupied Heating Setpoint | `int16s` | `int16_t` | Scene, Read / Write | `2000` (20.0 °C) | 0.01 °C | `500` | `3000` | Target temperature for heating. Writing a value outside the valid range will result in a `INVALID_VALUE` ZCL status. Must remain below Occupied Cooling Setpoint to avoid ZCL enforcement errors. |
| `0x001B` | Control Sequence Of Operation | `enum8` | `uint8_t` | Read / Write | `4` | - | - | - | Value `4` = Heating Only. |
| `0x001C` | System Mode | `enum8` | `uint8_t` | Scene, Read / Write | `0x04` (Heat) | - | - | - | Operating mode. Supported values: `0x00` Off, `0x04` Heat. Changing this attribute also synchronises `StelproSystemMode` (`0x401C`). |

#### 3.2 - Additional Standard Attributes

These attributes extend standard ZCL thermostat functionality and are added manually via `esp_zb_thermostat_cluster_add_attr()`.

| Attr ID | Name | ZCL Type | C Type | Access | Default | Unit | Notes |
|---|---|---|---|---|---|---|---|
| `0x0001` | Outdoor Temperature | `int16s` | `int16_t` | Read Only | `0` | 0.01 °C | ZCL standard outdoor temperature, intended for a physical sensor on the device. Not to be confused with the Stelpro custom outdoor-temperature display attribute (`0x4001`). |
| `0x0002` | Occupancy | `bitmap8` | `zb_uint8_t` | Read Only | `0x01` | - | Bit 0: `1` = occupied. Hardcoded to occupied since this is a fixed-installation heater. |
| `0x0008` | PI Heating Demand | `uint8` | `uint8_t` | Reporting, Read Only | `0` | % | Percentage of heating demand. Range: `0`–`100`. Must not be set to a non-zero value unless `running_state` has the HEAT bit set. Must be reset to `0` before clearing the HEAT bit. Zigbee2MQTT assumes range `[0, 255]`; this implementation uses `[0, 100]`. |
| `0x0029` | Thermostat Running State | `bitmap16` | `uint16_t` | Read Only | `0x0000` | - | `0x0000` = Idle; `0x0001` = Heat state ON (bit 0). |
| `0x0003` | Abs Min Heat Setpoint Limit | `int16s` | `int16_t` | Read Only | `500` (5.0 °C) | 0.01 °C | Absolute hardware minimum for the heating setpoint. |
| `0x0004` | Abs Max Heat Setpoint Limit | `int16s` | `int16_t` | Read Only | `3000` (30.0 °C) | 0.01 °C | Absolute hardware maximum for the heating setpoint. |
| `0x0015` | Min Heat Setpoint Limit | `int16s` | `int16_t` | Read / Write | `500` (5.0 °C) | 0.01 °C | Operator-adjustable minimum for the heating setpoint. |
| `0x0016` | Max Heat Setpoint Limit | `int16s` | `int16_t` | Read / Write | `3000` (30.0 °C) | 0.01 °C | Operator-adjustable maximum for the heating setpoint. |
| `0x0005` | Abs Min Cool Setpoint Limit | `int16s` | `int16_t` | Read Only | `500` (5.0 °C) | 0.01 °C | Present to satisfy ZCL schema even though the device is heating-only. |
| `0x0006` | Abs Max Cool Setpoint Limit | `int16s` | `int16_t` | Read Only | `3500` (35.0 °C) | 0.01 °C | Set above max heating setpoint to prevent ZCL enforcement conflicts. |
| `0x0017` | Min Cool Setpoint Limit | `int16s` | `int16_t` | Read / Write | `500` (5.0 °C) | 0.01 °C | Present to satisfy ZCL schema. |
| `0x0018` | Max Cool Setpoint Limit | `int16s` | `int16_t` | Read / Write | `3500` (35.0 °C) | 0.01 °C | Present to satisfy ZCL schema. |

#### 3.3 - Stelpro Manufacturer-Specific Attributes

These four attributes occupy the manufacturer-specific range (`0x4000`–`0x7FFF`) of cluster `0x0201`. They are registered using `esp_zb_cluster_add_attr()` rather than `esp_zb_cluster_add_manufacturer_attr()`, because registering them with a custom manufacturer code causes coordinators to reject read/write operations with `UNSUPPORTED_ATTRIBUTE`. Using the standard add function makes them accessible without requiring special manufacturer-code handling on the coordinator side.

| Attr ID | Z2M Attribute Name | ZCL Type | C Type | Access | Default | Unit | Min | Max | Notes |
|---|---|---|---|---|---|---|---|---|---|
| `0x4001` | `StelproOutdoorTemp` | `int16s` | `int16_t` | Read / Write | `0` | 0.01 °C | `−3200` (−32.0 °C) | `19900` (199.0 °C) | Outdoor temperature displayed on the thermostat face. This is a **display** value pushed by the coordinator - it does not reflect a physical sensor on the device. Defined by zigbee-herdsman as `{ID: 0x4001, type: DataType.INT16}`. |
| `0x401C` | `StelproSystemMode` | `enum8` | `uint8_t` | Scene, Read / Write | `0x04` (Heat) | - | - | - | Mirror of the standard `SystemMode` attribute (`0x001C`). Both carry identical semantics and are always kept in sync: writing either one causes the other to be updated immediately. Synchronisation is performed in software via `updateSystemModes()` since sharing a single `data_p` pointer in the ESP Zigbee SDK causes heap corruption at runtime. |
| `0x4008` | `StelproPower` | `uint16` | `uint16_t` | Reporting, Read Only | `0` | W | `0` | `4000` | Instantaneous electrical power draw of the baseboard heater. Updated at runtime from the heating demand calculation. Zigbee2MQTT exposes this value directly in Watts. |
| `0x4009` | `StelproEnergy` | `uint32` | `uint32_t` | Reporting, Read Only | `0` | Wh | `0` | `4 294 967 295` | Cumulative energy consumption since last reset. The on-wire value is in **Watt-hours (Wh)**. Zigbee2MQTT divides by `1000` before publishing, so Home Assistant receives the value in **kWh**. Accumulated by multiplying instantaneous power by the elapsed time between update cycles. |

---

### Cluster 4 - Thermostat UI Configuration (`0x0204`)

Controls the physical display and keypad behaviour of the thermostat.

#### Attributes

| Attr ID | Name | ZCL Type | C Type | Access | Default | Notes |
|---|---|---|---|---|---|---|
| `0xFFFD` | Cluster Revision | `uint16` | `uint16_t` | Read Only | `4` | Auto-added by SDK |
| `0x0000` | Temperature Display Mode | `enum8` | `uint8_t` | Read / Write | `0` | `0` = Celsius, `1` = Fahrenheit. |
| `0x0001` | Keypad Lockout | `enum8` | `uint8_t` | Read / Write | `0` | `0` = No lockout. Higher values lock progressively more buttons. Note: values written by the device itself may not be reflected in Zigbee2MQTT, as the coordinator may not subscribe to changes on this attribute. |


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

* The attribute represent a _Percentage of Heating Demand_. See macros `ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MIN_VALUE` & `ESP_ZB_ZCL_THERMOSTAT_PI_HEATING_DEMAND_MAX_VALUE` which defines the minimum and maximum values.
* As a percentage, it must be in range [0, 100]. However, _Zigbee2mqtt_ assume values are in [0,255] range. Other zigbee projects (including esp-zigbee-sdk library) assume values in [0,100] range.
* TODO: validate with a real Stelpro thermostat.


**Ordering Constraint**:

The `PIHeatingDemand` attribute (`0x0008`) has a strict ordering requirement enforced by the ZCL stack:

- The HEAT bit in `RunningState` (`0x0029`) **must be set before** `PIHeatingDemand` is set to a non-zero value.
- `PIHeatingDemand` **must be reset to `0` before** the HEAT bit is cleared from `RunningState`.

Violating this order results in a ZCL error. The simulation loop in `daikin-zigbee-bridge.ino` accounts for this ordering explicitly.

### StelproOutdoorTemp Registration Method

Custom attributes added with `esp_zb_cluster_add_manufacturer_attr()` using a non-`0xFFFF` manufacturer code are rejected by Zigbee2MQTT with `UNSUPPORTED_ATTRIBUTE`. `StelproOutdoorTemp` (`0x4001`) is therefore registered using the plain `esp_zb_cluster_add_attr()` function, which registers it without a manufacturer-specific code and makes it universally accessible.

### Thermostat UI Config, Keypad Lockout

* Can be set by the device itself but the new value won't be reflected in zigbee2mqtt. zigbee2mqtt might simple not watching the attribute for changes.
* TODO: validate with a real Stelpro thermostat.
