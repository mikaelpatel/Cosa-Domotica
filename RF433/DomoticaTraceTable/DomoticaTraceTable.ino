/**
 * @file DomoticaTraceTable.ino
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
 * Domotica demonstration sketch; Recieve messages, store in table and
 * print periodically.
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
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/RTC.hh"
#include "Cosa/Time.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Periodic.hh"

namespace Sensor {

  /**
   * Sensor data table with source node, port, message and timestamp
   * of latest update.
   */
  template <size_t NMEMB>
  class Table {
  public:
    Table();
    bool update(uint8_t src, uint8_t port, Domotica::msg_t& msg);
    void print(IOStream& outs);

  private:
    struct sensor_t {
      uint32_t timestamp;
      uint8_t src;
      uint8_t port;
      Domotica::msg_t msg;
    };

    sensor_t m_sensor[NMEMB];
  };
};

template<size_t NMEMB>
Sensor::Table<NMEMB>::Table()
{
  memset(m_sensor, 0, sizeof(m_sensor));
}

template<size_t NMEMB>
bool
Sensor::Table<NMEMB>::update(uint8_t src, uint8_t port, Domotica::msg_t& msg)
{
  for (int i = 0; i < NMEMB; i++) {
    if (m_sensor[i].src == src
	&& m_sensor[i].port == port
	&& m_sensor[i].msg.id == msg.id)
      {
	m_sensor[i].timestamp = RTC::seconds();
	m_sensor[i].msg = msg;
	return (true);
      }
    if (m_sensor[i].src == 0) {
      m_sensor[i].timestamp = RTC::seconds();
      m_sensor[i].src = src;
      m_sensor[i].port = port;
      m_sensor[i].msg = msg;
      return (true);
    }
  }
  return (false);
}

template<size_t NMEMB>
void
Sensor::Table<NMEMB>::print(IOStream& outs)
{
  uint32_t now = RTC::seconds();
  outs << time_t(now) << endl;
  for (int i = 0; i < NMEMB; i++) {
    if (m_sensor[i].src == 0) break;
    uint8_t src = m_sensor[i].src;
    uint8_t port = m_sensor[i].port;
    Domotica::msg_t* msg = &m_sensor[i].msg;
    time_t timestamp = m_sensor[i].timestamp;
    uint32_t age = now - timestamp;
    outs << time_t(timestamp) << PSTR(":T-") << age << PSTR(":sensor=");
    Domotica::print(outs, src, port, msg->id);
    outs << PSTR(",nr=") << msg->nr
	 << PSTR(",vcc=") << msg->battery
	 << PSTR(":");
    Domotica::print(outs, port, msg);
    outs << endl;
  }
  outs << endl;
}

const int TABLE_MAX = 8;
Sensor::Table<TABLE_MAX> table;

void setup()
{
  Domotica::begin(&rf);
  time_t::epoch_year(2015);
  uart.begin(57600);
  trace.begin(&uart, PSTR("DomoticaTraceTable: started"));
  rf.powerup();
}

void loop()
{
  const uint32_t TIMEOUT = 1000L;
  Domotica::msg_t msg;
  uint8_t src;
  uint8_t port;
  int res = rf.recv(src, port, &msg, sizeof(msg), TIMEOUT);
  if (res > 0) {
    if (!table.update(src, port, msg)) {
      trace << PSTR("warning: sensor table full") << endl;
    }
  }

  periodic(10000) table.print(trace);
}
