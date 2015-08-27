/**
 * @file DomoticaRelay.ino
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
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x02
#define DEST 0x01

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"

#if !defined(BOARD_ATTINY)
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#else
#define ASSERT(x)
#endif

void setup()
{
#if !defined(BOARD_ATTINY)
  uart.begin(9600);
  trace.begin(&uart, PSTR("DomoticaRelay: started"));
#endif
  Watchdog::begin();
  RTC::begin();
  ASSERT(rf.begin());
}

void loop()
{
  const uint32_t TIMEOUT = 10000;
  Domotica::msg_t msg;
  uint8_t src;
  uint8_t port;
  int count = rf.recv(src, port, &msg, sizeof(msg), TIMEOUT);
  if (count < 0) {
#if !defined(BOARD_ATTINY)
    trace << PSTR("recv:error(") << count << PSTR(")\n");
#endif
    return;
  }
  if (!rf.is_broadcast()) return;

#if !defined(BOARD_ATTINY)
  trace << RTC::millis()
	<< PSTR(":src=") << hex << src
	<< PSTR(",port=") << hex << port
	<< PSTR(",dest=") << rf.get_device_address()
	<< endl;
#endif
  count = rf.send(DEST, port, &msg, count);
#if !defined(BOARD_ATTINY)
  if (count < 0) {
    trace << PSTR("send:error(") << count << PSTR(")\n");
  }
#endif
}
