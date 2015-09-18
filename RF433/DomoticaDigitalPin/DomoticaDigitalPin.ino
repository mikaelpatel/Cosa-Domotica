/**
 * @file DomoticaDigitalPin.ino
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
 * Domotica demonstation sketch; Periodically sample digital pins
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
 * (D0)------------------<
 *   - ------------------<
 * (Dn)------------------<
 *
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x10

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI::Transmitter tx(TX, &codec);
VWI rf(NETWORK, DEVICE, SPEED, &tx);

// Sketch includes
#include "Cosa/OutputPin.hh"
#include "Cosa/InputPin.hh"

// Flash led during transmission
OutputPin led(Board::LED, 0);

void setup()
{
  // Start timers and wireless driver
  Domotica::begin(&rf);
}

void loop()
{
  // Sequence number and pin index
  static uint8_t nr = 0;
  static uint8_t ix = 0;

  // Construct the message with pin index and state
  Domotica::DigitalPin::msg_t msg;
  Board::DigitalPin pin;
  pin = (Board::DigitalPin) pgm_read_byte(&digital_pin_map[ix]);
  msg.set(nr, ix);
  msg.value = InputPin::read(pin);

  // Next pin index
  ix = (ix + 1);
  if (ix == membersof(digital_pin_map)) ix = 0;

  // Boardcast message
  led.on();
  rf.powerup();
  rf.broadcast(Domotica::DIGITAL_PIN_MSG, &msg, sizeof(msg));
  rf.powerdown();
  led.off();

  // Deep sleep with only watchdog awake
  Domotica::sleep();
}
