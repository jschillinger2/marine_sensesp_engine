#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp_app_builder.h"
#include "sensesp_onewire/onewire_temperature.h"

using namespace sensesp;

ReactESP app;

void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  /*
     Find all the sensors and their unique addresses. Then, each new instance
     of OneWireTemperature will use one of those addresses. You can't specify
     which address will initially be assigned to a particular sensor, so if you
     have more than one sensor, you may have to swap the addresses around on
     the configuration page for the device. (You get to the configuration page
     by entering the IP address of the device into a browser.)
  */


/**
 * Heat Sensors
*/

  /*
     Tell SensESP where the sensor is connected to the board
     ESP32 pins are specified as just the X in GPIOX
  */
  uint8_t pin = 4;

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(pin);

  // Define how often SensESP should read the sensor(s) in milliseconds
  uint read_delay = 500;

  // Below are temperatures sampled and sent to Signal K server
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html

  // Measure coolant temperature
  auto* coolant_temp =
      new OneWireTemperature(dts, read_delay, "/coolantTemperature/oneWire");

  coolant_temp->connect_to(new Linear(1.0, 0.0, "/coolantTemperature/linear"))
      ->connect_to(new SKOutputFloat("propulsion.mainEngine.coolantTemperature",
                                     "/coolantTemperature/skPath"));

  // Measure exhaust temperature
  auto* exhaust_temp =
      new OneWireTemperature(dts, read_delay, "/exhaustTemperature/oneWire");

  exhaust_temp->connect_to(new Linear(1.0, 0.0, "/exhaustTemperature/linear"))
      ->connect_to(new SKOutputFloat("propulsion.mainEngine.exhaustTemperature",
                                     "/exhaustTemperature/skPath"));

  // Measure temperature of 12v alternator
  auto* alt_12v_temp =
      new OneWireTemperature(dts, read_delay, "/12vAltTemperature/oneWire");

  alt_12v_temp->connect_to(new Linear(1.0, 0.0, "/12vAltTemperature/linear"))
      ->connect_to(new SKOutputFloat("electrical.alternators.12V.temperature",
                                     "/12vAltTemperature/skPath"));

/**
 * Seacock Sensor
*/



// Create another digital input, this time with RepeatSensor. This approach
  // can be used to connect external sensor library to SensESP!

  const uint8_t kDigitalInput2Pin = 14;
  const unsigned int kDigitalInput2Interval = 1000;

  // Configure the pin. Replace this with your custom library initialization
  // code!
  pinMode(kDigitalInput2Pin, INPUT_PULLUP);

  // Define a new RepeatSensor that reads the pin every 100 ms.
  // Replace the lambda function internals with the input routine of your custom
  // library.

  // Again, test this yourself by connecting pin 15 to pin 13 with a jumper
  // wire and see if the value changes!

  auto* digital_input2 = new RepeatSensor<bool>(
      kDigitalInput2Interval,
      [kDigitalInput2Pin]() { return digitalRead(kDigitalInput2Pin); });


// Connect digital input 2 to Signal K output.
  digital_input2->connect_to(new SKOutputBool(
      "sensors.digital_input2.value",          // Signal K path
      "/sensors/digital_input2/value",         // configuration path
      new SKMetadata("",                       // No units for boolean values
                     "Seacock Open")  // Value description
      ));



  // Configuration is done, lets start the readings of the sensors!
  sensesp_app->start();
}

// main program loop
void loop() { app.tick(); }