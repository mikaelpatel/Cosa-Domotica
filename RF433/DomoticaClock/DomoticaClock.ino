/**
 * @file DomoticaClock.ino
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
 * Domotica demonstation sketch; Broadcast real-time clock message
 * every 10 seconds.
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
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x60
#define ID 0x00

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI::Transmitter tx(TX, &codec);
VWI rf(NETWORK, DEVICE, SPEED, &tx);

// Sketch includes
#include "Cosa/RTT.hh"
#include "Cosa/OutputPin.hh"

// Wall-clock
RTT::Clock clock;

// Flash led during transmission
OutputPin led(Board::LED, 0);

void setup()
{
  // Start timers and wireless driver
  Domotica::begin(&rf);
}

void loop()
{
  // Sequence number
  static uint8_t nr = 0;

  // Construct the message with clock
  Domotica::RealTimeClock::msg_t msg;
  clock_t now = clock.time();
  msg.set(nr, ID);
  msg.time = now;

  // Boardcast message
  led.on();
  rf.powerup();
  rf.broadcast(Domotica::REALTIME_CLOCK_MSG, &msg, sizeof(msg));
  rf.powerdown();
  led.off();

  // Send every 10 seconds
  while (clock.time() - now < 10_s) yield();
}
