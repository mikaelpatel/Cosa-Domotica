/**
 * @file DomoticaInfo.ino
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
 * Domotica demonstation sketch; Periodically send information
 * to the DomoticaTrace (DEST).
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
#define DEVICE 0x80
#define ID 0x00
#define DEST 0x01

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

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

  // Construct the message with information string
  Domotica::InfoString::msg_t msg;
  size_t len;
  msg.set(nr, ID);
  strcpy_P(msg.info, PSTR("Still alive"));
  len = sizeof(Domotica::header_t) + strlen(msg.info) + 1;

  // Send message to given destination
  led.on();
  rf.powerup();
  rf.send(DEST, Domotica::INFO_STRING_MSG, &msg, len);
  rf.powerdown();
  led.off();

  // Deep sleep with only watchdog awake
  Domotica::sleep();
}
