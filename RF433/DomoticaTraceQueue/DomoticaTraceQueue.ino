/**
 * @file DomoticaTraceQueue.ino
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
 * Domotica demonstration sketch; Recieve messages, queue and print
 * periodically.

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
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/RTC.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Periodic.hh"
#include "Cosa/Queue.hh"

// Sensor data
struct sensor_t {
  uint8_t src;
  uint8_t port;
  Domotica::msg_t msg;
  uint32_t timestamp;

  sensor_t(uint8_t src, uint8_t port, Domotica::msg_t& msg)
  {
    this->src = src;
    this->port = port;
    this->msg = msg;
    this->timestamp = RTC::seconds();
  }
  sensor_t() {}
};

const int SENSOR_MAX = 8;
Queue<sensor_t, SENSOR_MAX> queue;

void setup()
{
  Domotica::begin(&rf);
  uart.begin(57600);
  trace.begin(&uart, PSTR("DomoticaTraceQueue: started"));
  rf.powerup();
}

void loop()
{
  // Receive message from sensor and queue
  const uint32_t TIMEOUT = 1000L;
  Domotica::msg_t msg;
  uint8_t src;
  uint8_t port;
  int res = rf.recv(src, port, &msg, sizeof(msg), TIMEOUT);
  if (res > 0) {
    sensor_t sensor(src, port, msg);
    if (!queue.enqueue(&sensor)) {
      trace << PSTR("warning: queue full") << endl;
    }
  }

  // Periodically print the queued sensor messages
  periodic(10000) {
    sensor_t sensor;
    while (queue.dequeue(&sensor)) {
      trace << time_t(sensor.timestamp) << PSTR(":sensor=");
      Domotica::print(trace, sensor.src, sensor.port, sensor.msg.id);
      trace << PSTR(",nr=") << sensor.msg.nr
	    << PSTR(",vcc=") << sensor.msg.battery
	    << PSTR(":");
      Domotica::print(trace, sensor.port, &sensor.msg);
      trace << endl;
    }
  }
}
