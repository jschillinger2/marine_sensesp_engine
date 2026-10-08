#pragma once
// app_config.h – Editable device properties
// -------------------------------------------------------------
// Modify the constants below, then re‑compile & flash.
// -------------------------------------------------------------

#include <vector>

/************* Network *************/
constexpr const char* kWifiSSID       = "xyz";   // Wi‑Fi SSID
constexpr const char* kWifiPassword   = "xyz";   // Wi‑Fi password

/********* Signal K Server *********/
constexpr const char*  kSKServerIP    = "";        // e.g. "10.10.7.16"
                                                   // Leave kSKServerIP empty ("") for auto‑discovery via mDNS.
constexpr const uint16_t kSKServerPort = 3000;     // usual SK port

/************* Temp Sensors **************/
constexpr uint8_t kTempSensorPin = 4;        // 1‑Wire data pin
constexpr uint16_t kTempReadDelay = 500;     // ms between temp samples

/************* RPM Sensor **************/
constexpr uint8_t kRpmSensorPin = 17;
constexpr uint16_t kRpmReadDelay = 500;
constexpr float multiplier = 1.0 / 12.38;
