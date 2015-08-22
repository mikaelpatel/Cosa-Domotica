# Cosa-Domotica

This Cosa library is a framework for Home Automation (Domotica) with
cheap RF433 transmitter and receiver modules.

Domotica supports the following messages with sensor data:
* [Analog Pin] (./RF433/DomoticaAnalogPin)
* [Digital Pin] (./RF433/DomoticaDigitalPin)
* [Digital Pins] (./RF433/DomoticaDigitalPins)
* [DHTXX Sensor] (./RF433/DomoticaDHT11)
* [DS18B20 Sensor] (./RF433/DomoticaDS18B20)

The demonstation sketches for monitoring messages are:
* [LCD] (./RF433/DomoticaMonitor)
* [Trace] (./RF433/DomoticaTrace)

The framework supports low power mode with power down sleep or
external interrupt await.
* [Wakeup] (./RF433/DomoticaWakeup)

# Install

To use this library you must download and install the [Arduino IDE] (http://www.arduino.cc/en/Main/Software) (or
GCC AVR toolchain) and [Cosa] (https://github.com/mikaelpatel/Cosa).

Download and unzip the Cosa-Domotica library into your sketchbook
libraries directory. Rename from Cosa-Domotica-master to
Domotica.

The Domotica library and examples should be found in the Arduino IDE
File>Sketchbook menu. Open the DomoticaMonitor example sketch. Select
the Cosa core by selecting one of the Cosa boards in the Tools>Board
menu.


