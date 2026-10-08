// main.cpp – Signal K Battery Control (no config‑paths)

#include <memory>
#include <cstring>
#include <vector>
#include <algorithm>

#include "sensesp.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"
#include "sensesp/transforms/frequency.h"

#include "sensesp/transforms/linear.h"
#include "sensesp_onewire/onewire_temperature.h"

#include <Wire.h>
#include <INA219_WE.h>

#include "app_config.h"

using namespace sensesp;
using namespace sensesp::onewire;

INA219_WE *ina219; // will be created in setupCurrentSensor()

// ──────────────────────────────────────────────────────────────
void setupTempSensors();
void setupRPM();
// ──────────────────────────────────────────────────────────────

void setup()
{
  SetupLogging(ESP_LOG_DEBUG);
  debugI("Started App");

  SensESPAppBuilder base_builder;
  auto *builder = base_builder.set_hostname("EngineControl");

  if (strlen(kWifiSSID) > 0)
  {
    debugI("Setting WiFi credentials - SSID: %s, Password: %s", kWifiSSID, kWifiPassword);
    builder->set_wifi_client(kWifiSSID, kWifiPassword);
  }
  else
    debugI("No WiFi SSID configured");

  if (strlen(kSKServerIP) > 0)
  {
    debugI("Setting KServer Manual to IP: %s, Port: %d", kSKServerIP, kSKServerPort);
    builder->set_sk_server(kSKServerIP, kSKServerPort);
  }

  sensesp_app = builder->get_app();

  setupTempSensors();
  setupRPM();

  sensesp_app->start();
}

void setupRPM()
{

  debugI("Setting up RPM Sensors - START");

  // The "Signal K path" identifies the output of the sensor to the Signal K
  // network.
  const char *sk_path = "propulsion.main.revolutions";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  const char *config_path = "/sensors/engine_rpm";

  // These two are necessary until a method is created to synthesize them.
  // Everything after "/sensors" in each of these ("/engine_rpm/calibrate" and
  // "/engine_rpm/sk") is simply a label to display what you're configuring in
  // the Configuration UI.
  const char *config_path_calibrate = "/sensors/engine_rpm/calibrate";
  const char *config_path_skpath = "/sensors/engine_rpm/sk";

  //////////
  // connect a RPM meter. A DigitalInputPcntCounter implements
  // access to a pulse counter peripheral. Every read_delay ms, it
  // reads the number of pulses that have occurred since the last
  // read, and reports that number (500ms in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.
  // const float multiplier = 1.0 / 12.38;
  //const unsigned int read_delay = 500;

  auto *sensor = new DigitalInputCounter(kRpmSensorPin, INPUT_PULLUP, RISING, kRpmReadDelay);

  auto frequency = new Frequency(multiplier, config_path_calibrate);

  ConfigItem(frequency)
      ->set_title("Frequency")
      ->set_description("Frequency of the engine RPM signal")
      ->set_sort_order(1000);

  // Metadata lets consumers render a gauge with a sensible range and raise
  // an overspeed alarm. displayScale needs both bounds; keep zones
  // non-overlapping so every consumer resolves them the same way.
  auto *metadata = new SKMetadata("Hz", "Engine RPM");
  metadata->display_scale_lower_ = 0.0f;
  metadata->display_scale_upper_ = 120.0f;
  metadata->zones_.push_back(
      SKMetadataZone(SKAlarmState::kNominal, "Normal range", 10.0f, 100.0f));
  metadata->zones_.push_back(
      SKMetadataZone(SKAlarmState::kAlarm, "Overspeed", 110.0f));

  auto frequency_sk_output =
      new SKOutput<float>(sk_path, config_path_skpath, metadata);

  ConfigItem(frequency_sk_output)
      ->set_title("Frequency SK Output Path")
      ->set_sort_order(1001);

  sensor
      ->connect_to(frequency)            // connect the output of sensor
                                         // to the input of Frequency()
      ->connect_to(frequency_sk_output); // connect the output of Frequency()
                                         // to a Signal K Output as a number

  // Add this block to log the signal detection
  frequency->attach([frequency]()
                    {
    float current_hz = frequency->get();
    // Optional: Only log if the engine is actually turning (Hz > 0)
    if (current_hz > 0.0) {
        debugD("Propulsion signal detected! Current frequency: %.2f Hz", current_hz);
    } });

  debugI("Setting up RPM Sensors - STOP");
}

// ──────────────────────────────────────────────────────────────
void setupTempSensors()
{
  debugI("Setting up Temp Sensors - START");

  DallasTemperatureSensors *dts = new DallasTemperatureSensors(kTempSensorPin);
  uint32_t read_delay = 500;

  {
    auto coolant_temp = new OneWireTemperature(dts, read_delay, "/coolantTemperature/oneWire");
    coolant_temp
        ->connect_to(new Linear(1.0, 0.0, "/coolantTemperature/linear"))
        ->connect_to(new SKOutputFloat("propulsion.mainEngine.coolantTemperature", ""));
    coolant_temp->attach([coolant_temp]
                         { debugD("Coolant temp: %.1f °C", coolant_temp->get()); });
  }

  {
    auto alternator_temp = new OneWireTemperature(dts, read_delay, "/alternatorTemperature/oneWire");
    alternator_temp
        ->connect_to(new Linear(1.0, 0.0, "/alternatorTemperature/linear"))
        ->connect_to(new SKOutputFloat("propulsion.mainEngine.alternatorTemperature", ""));
    alternator_temp->attach([alternator_temp]
                            { debugD("Alternator temp: %.1f °C", alternator_temp->get()); });
  }

  {
    auto exaust_temp = new OneWireTemperature(dts, read_delay, "/exaustTemperature/oneWire");
    exaust_temp
        ->connect_to(new Linear(1.0, 0.0, "/exaustTemperature/linear"))
        ->connect_to(new SKOutputFloat("propulsion.mainEngine.exaustTemperature", ""));
    exaust_temp->attach([exaust_temp]
                        { debugD("Exaust temp: %.1f °C", exaust_temp->get()); });
  }

  debugI("Setting up Temp Sensors - STOP");
}

// ──────────────────────────────────────────────────────────────
void loop() { event_loop()->tick(); }
// ──────────────────────────────────────────────────────────────
