/**
 * @file DomoticaDS18B20.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2015, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * @section Description
 * Domotica demonstation sketch; Periodically sample 1-Wire DS18B20
 * thermometer and send message.
 *
 * @section Circuit
 * @code
 *                         RF433/TX
 *                       +------------+
 * (D6/D0)-------------1-|DATA        |
 * (VCC)---------------2-|VCC         |                    V
 * (GND)---------------3-|GND         |                    |
 *                       |ANT       0-|--------------------+
 *                       +------------+       17.3 cm
 *
 *                       DS18B20/sensor
 *                       +------------+
 * (GND)---------------1-|GND         |\
 * (OWI/D3)--+---------2-|DQ          | |
 *           |       +-3-|VDD         |/
 *          4K7      |   +------------+
 *           |       |
 * (PW/D4)---+       +---(VCC/GND)
 *
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x40
#define ID 0x00

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/OutputPin.hh"

#include <OWI.h>
#include <DS18B20.h>

// Connect to one-wire device
OWI owi(Board::D3);
DS18B20 thermometer(&owi);

// Active pullup (pullup resistor 4K7 connected between this pin
// and OWI pin)
OutputPin pw(Board::D4);

void setup()
{
  // Startup timers and wireless device
  Domotica::begin(&rf);

  // Connect to thermometer
  asserted(pw) thermometer.connect(ID);
}

void loop()
{
  // Message sequence number initialization
  static uint8_t nr = 0;

  // Make a conversion request and read the temperature (scratchpad)
  asserted(pw) {
    thermometer.convert_request();
    thermometer.read_scratchpad();
  }

  // Initiate the message with measurements
  Domotica::Thermometer::msg_t msg;
  msg.set(nr, ID);
  msg.temperature = thermometer.get_temperature() * 0.0625;

  // Broadcast the message and power down after completion
  rf.powerup();
  rf.broadcast(Domotica::THERMOMETER_MSG, &msg, sizeof(msg));
  rf.powerdown();

  // Deep sleep with only the watchdog awake
  Domotica::sleep();
}

