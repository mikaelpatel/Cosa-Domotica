/**
 * @file DomoticaTrace.ino
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
 * Domotica demonstration sketch; Recieve messages and print to UART.
 *
 * @section Circuit
 * @code
 *                         RF433/RX                       V
 *                       +------------+                   |
 *                       |0-----------|-------------------+
 *                       |ANT         |       17.3 cm
 *                       |            |
 *                       |            |
 *                       |            |
 *                       |            |
 * (VCC)---------------1-|VCC         |
 *                     2-|DATA        |
 * (RX/D7)-------------3-|DATA        |
 * (GND)---------------4-|GND         |
 *                       +------------+
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x01

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI::Receiver rx(RX, &codec);
VWI rf(NETWORK, DEVICE, SPEED, &rx);

// Sketch includes
#include "Cosa/RTT.hh"
#include "Cosa/Time.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/InputPin.hh"

// Wall-clock
RTT::Clock clock;

void setup()
{
  Domotica::begin(&rf);
  time_t::epoch_year(2015);
  uart.begin(9600);
  trace.begin(&uart, PSTR("DomoticaTrace: started"));
  rf.powerup();
}

void loop()
{
  const uint32_t TIMEOUT = 60000L;
  Domotica::msg_t msg;
  uint8_t src;
  uint8_t port;
  int res = rf.recv(src, port, &msg, sizeof(msg), TIMEOUT);
  if (res < 0) return;
  int rssi = rf.get_link_quality_indicator();
  uint32_t now = clock.time();
  trace << time_t(now) << PSTR(":sensor=");
  Domotica::print(trace, src, port, msg.id);
  trace << PSTR(",nr=") << msg.nr
	<< PSTR(",vcc=") << msg.battery
	<< PSTR(",rssi=") << rssi
	<< PSTR(",len=") << res
	<< PSTR(":");
  Domotica::print(trace, port, &msg);
  trace << endl;
}
