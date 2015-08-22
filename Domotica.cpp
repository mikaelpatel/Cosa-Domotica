/**
 * @file Domotica.cpp
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
 * This file is part of the Arduino Che Cosa project.
 */

#include "Domotica.hh"

#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/AnalogPin.hh"
#include "Cosa/ExternalInterrupt.hh"

bool
Domotica::begin(Wireless::Driver* rf, void* config)
{
  Watchdog::begin();
  RTC::begin();
  return (rf->begin(config));
}

void
Domotica::sleep(uint16_t s)
{
  uint32_t ms = s * 1000L;
  Power::all_disable();
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
  Watchdog::delay(ms);
  Power::set(mode);
  Power::all_enable();
  Watchdog::delay(16);
}

class Button : public ExternalInterrupt {
public:
  Button(Board::ExternalInterruptPin pin) :
    ExternalInterrupt(pin, ON_LOW_LEVEL_MODE, true)
  {}

  virtual void on_interrupt(uint16_t arg)
  {
    UNUSED(arg);
    disable();
  }
};

void
Domotica::await(Board::ExternalInterruptPin pin)
{
  Button wakeup(pin);

  Power::all_disable();
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
  do Watchdog::delay(128); while (wakeup.is_clear());
  Watchdog::end();
  RTC::end();
  wakeup.enable();

  while (wakeup.is_set()) yield();

  Power::set(mode);
  Watchdog::begin();
  RTC::begin();
  Power::all_enable();
  ::AnalogPin::powerup();
}

IOStream& operator<<(IOStream& outs, Domotica::header_t* header)
{
  outs << header->battery << PSTR(" mV,")
       << header->nr;
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::msg_t* msg)
{
  outs.print(0L, msg->payload, Domotica::PAYLOAD_MAX, IOStream::hex);
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::DigitalPin::msg_t* msg)
{
  outs << PSTR("D[") << msg->id << PSTR("]:");
  if (msg->value)
    outs << PSTR("on");
  else
    outs << PSTR("off");
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::DigitalPins::msg_t* msg)
{
  uint8_t pins = msg->id & (Domotica::DigitalPins::MAX - 1);
  outs << PSTR("D[0..") << pins - 1 << PSTR("]:");
  outs.print(msg->value, pins, IOStream::bin);
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::AnalogPin::msg_t* msg)
{
  outs << PSTR("A[") << msg->id << PSTR("]:")
       << msg->value;
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::DS18B20::msg_t* msg)
{
  outs << PSTR("DS18B20[") << msg->id << PSTR("]:");
  int16_t temp = msg->temperature;
  if (temp < 0) {
    temp = -temp;
    outs << '-';
  }
  uint16_t fraction = (625 * (temp & 0xf)) / 100;
  int16_t integer = (temp >> 4);
  outs << integer << '.';
  if (fraction < 10) outs << '0';
  outs << fraction << PSTR(" C");
  return (outs);
}

IOStream& operator<<(IOStream& outs, Domotica::DHT::msg_t* msg)
{
  outs << PSTR("DHT[") << msg->id << PSTR("]:");
  outs << msg->humidity / 10 << '.' << msg->humidity % 10 << PSTR(" %,")
       << msg->temperature / 10 << '.' << msg->temperature % 10 << PSTR(" C");
  return (outs);
}

