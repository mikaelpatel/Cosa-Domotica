/**
 * @file DomoticaDHT11.ino
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
 * Domotica demonstation sketch; Periodically sample DHT11 sensor
 * and send message.
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
 *                        DHT11/sensor
 *                       +------------+
 * (VCC)---------------1-|VCC  ====== |
 * (EXT0)--------------2-|DATA ====== |
 *                     3-|     ====== |
 * (GND)---------------4-|GND  ====== |
 *                       +------------+
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x50
#define ID 0x00

// RF433 includes; Virtual Wire Wireless Interface and Huffman(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/OutputPin.hh"
#include <DHT.h>

// DHT pin configuration
#define EXT Board::EXT0

// Digital Humidity and Temperature Sensor Driver
DHT11 sensor(EXT);

// Flash led during transmission
OutputPin led(Board::LED, 0);

void setup()
{
  // Start timers and wireless driver
  Domotica::begin(&rf);
}

void loop()
{
  // Message sequence number
  static uint8_t nr = 0;

  // Construct message with humidity and temperature
  Domotica::DHT::msg_t msg;
  msg.header.set(nr, ID);
  sensor.sample(msg.humidity, msg.temperature);

  // Broadcast message and powerdown
  led.on();
  rf.powerup();
  rf.broadcast(Domotica::DHT_SENSOR_MSG, &msg, sizeof(msg));
  rf.powerdown();
  led.off();

  // Deep sleep with only watchdog awake
  Domotica::sleep();
}
