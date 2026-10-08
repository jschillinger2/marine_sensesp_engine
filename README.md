# EngineControl – Signal K ESP32 Sensor Interface

`EngineControl` is a SensESP-based ESP32 firmware designed to monitor marine engine metrics—including engine RPM and multiple OneWire temperature sensors—and publish them directly to a Signal K server.

---

## Features

* **Engine RPM Monitoring**: Reads pulse frequency from a digital input tachometer and outputs engine revolutions to Signal K with custom ranges and metadata alarms.
* **OneWire Temperature Monitoring**: Supports multiple 1-Wire Dallas temperature sensors on a single bus to monitor:
  * Main Engine Coolant Temperature (`propulsion.mainEngine.coolantTemperature`)
  * Main Engine Alternator Temperature (`propulsion.mainEngine.alternatorTemperature`)
  * Main Engine Exhaust Temperature (`propulsion.mainEngine.exaustTemperature`)
* **Signal K Integration**: Built on SensESP (v3.x) for seamless auto-discovery (mDNS) or direct IP server configuration.
* **Web UI Configuration**: Integrates configurable paths for calibration and output settings reachable via the SensESP web dashboard.

---

## Hardware Pin Mapping & Configuration

Hardcoded defaults and editable parameters can be updated in `app_config.h`:

| Function | Default Pin | Configuration Parameter | Notes |
| :--- | :--- | :--- | :--- |
| **Wi-Fi SSID** | - | `kWifiSSID` | Set to your Wi-Fi network SSID |
| **Wi-Fi Password** | - | `kWifiPassword` | Set to your Wi-Fi password |
| **Signal K Server IP** | - | `kSKServerIP` | Leave empty `""` for mDNS auto-discovery |
| **Signal K Server Port**| - | `kSKServerPort` | Default: `3000` |
| **1-Wire Data Pin** | GPIO 4 | `kTempSensorPin` | Data pin for Dallas temperature sensors |
| **RPM Input Pin** | GPIO 17 | `kRpmSensorPin` | Digital pulse counter pin |

---

## Dependencies & Requirements

* **Board**: ESP32 (`esp32dev`)
* **Framework**: Arduino / ESP-IDF
* **PlatformIO Libraries**:
  * `SignalK/SensESP` (`>=3.0.0-beta.6, <4.0.0-alpha.1`)
  * `SensESP/OneWire` (`^3.0.1`)
  * `INA219_WE`

---

## Getting Started

1. **Clone the repository** into your PlatformIO workspace.
2. **Configure your network settings** in `app_config.h`:
   ```cpp
   constexpr const char* kWifiSSID     = "Your_WiFi_SSID";
   constexpr const char* kWifiPassword = "Your_WiFi_Password";
   ```
3. **Connect hardware**:
   * Connect your 1-Wire temperature sensor bus to **GPIO 4**.
   * Connect your RPM pulse signal source to **GPIO 17**.
4. **Build & Upload**:
   Use PlatformIO to build and flash the firmware to your ESP32 board:
   ```bash
   pio run --target upload
   ```
5. **Monitor Serial Output**:
   ```bash
   pio device monitor
   ```