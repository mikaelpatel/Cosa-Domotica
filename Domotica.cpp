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

#include "Cosa/RTT.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/AnalogPin.hh"
#include "Cosa/ExternalInterrupt.hh"

bool
Domotica::begin(Wireless::Driver* rf, void* config)
{
  Watchdog::begin();
  RTT::begin();
  if (UNLIKELY(!rf->begin(config))) return (false);
  rf->powerdown();
  return (true);
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

void
Domotica::await(Board::ExternalInterruptPin pin,
		ExternalInterrupt::InterruptMode mode)
{
  InterruptPin intr(pin, mode);

  Power::all_disable();
  uint8_t old_sleep_mode = Power::set(SLEEP_MODE_PWR_DOWN);
  if (mode == ExternalInterrupt::ON_LOW_LEVEL_MODE)
    do Watchdog::delay(128); while (intr.is_clear());
  Watchdog::end();
  RTT::end();

  intr.enable();
  if (mode == ExternalInterrupt::ON_RISING_MODE)
    while (intr.is_clear()) yield();
  else
    while (intr.is_set()) yield();

  Power::set(old_sleep_mode);
  Watchdog::begin();
  RTT::begin();
  Power::all_enable();
  ::AnalogPin::powerup();
}

void
Domotica::print(IOStream& outs, uint8_t device, uint8_t port, uint8_t id)
{
  outs.print((uint16_t) device, 2, IOStream::hex);
  outs.print('.');
  outs.print((uint16_t) port, 2, IOStream::hex);
  outs.print('.');
  outs.print((uint16_t) id, 2, IOStream::hex);
}

void
Domotica::print(IOStream& outs, uint16_t network, uint8_t device, uint8_t port, uint8_t id)
{
  outs.print(network, 4, IOStream::hex);
  outs.print('.');
  outs.print((uint16_t) device, 2, IOStream::hex);
  outs.print('.');
  outs.print((uint16_t) port, 2, IOStream::hex);
  outs.print('.');
  outs.print((uint16_t) id, 2, IOStream::hex);
}

void
Domotica::print(IOStream& outs, uint8_t port, Domotica::msg_t* msg)
{
  switch (port) {
  case Domotica::INFO_STRING_MSG:
    outs << (Domotica::InfoString::msg_t*) msg;
    break;
  case Domotica::DIGITAL_PIN_MSG:
    outs << (Domotica::DigitalPin::msg_t*) msg;
    break;
  case Domotica::DIGITAL_PINS_MSG:
    outs << (Domotica::DigitalPins::msg_t*) msg;
    break;
  case Domotica::ANALOG_PIN_MSG:
    outs << (Domotica::AnalogPin::msg_t*) msg;
    break;
  case Domotica::THERMOMETER_MSG:
    outs << (Domotica::Thermometer::msg_t*) msg;
    break;
  case Domotica::HUMIDITY_TEMPERATURE_SENSOR_MSG:
    outs << (Domotica::HumidityTemperatureSensor::msg_t*) msg;
    break;
  case Domotica::REALTIME_CLOCK_MSG:
    outs << (Domotica::RealTimeClock::msg_t*) msg;
    break;
  case Domotica::ACCELEROMETER_MSG:
    outs << (Domotica::Accelerometer::msg_t*) msg;
    break;
  default:
    outs.print((uint32_t) msg, msg, sizeof(Domotica::msg_t), IOStream::hex);
  }
}

void
Domotica::print(IOStream& outs, uint8_t src, uint8_t port, Domotica::msg_t* msg)
{
  Domotica::print(outs, src, port, msg->id);
  outs << ':';
  Domotica::print(outs, port, msg);
}

IOStream&
operator<<(IOStream& outs, Domotica::header_t* header)
{
  outs << header->battery << PSTR(" mV,")
       << header->nr;
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::msg_t* msg)
{
  outs.print(0L, msg->payload, Domotica::PAYLOAD_MAX, IOStream::hex);
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::InfoString::msg_t* msg)
{
  outs << msg->info;
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::DigitalPin::msg_t* msg)
{
  if (msg->value)
    outs << PSTR("on");
  else
    outs << PSTR("off");
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::DigitalPins::msg_t* msg)
{
  uint8_t pins = msg->id & (Domotica::DigitalPins::MAX - 1);
  outs.print(msg->value, pins, IOStream::bin);
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::AnalogPin::msg_t* msg)
{
  outs << msg->value;
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::Thermometer::msg_t* msg)
{
  uint8_t old_precision = outs.precision(2);
  uint8_t old_width = outs.width(-5);
  outs << msg->temperature << PSTR(" C");
  outs.width(old_width);
  outs.precision(old_precision);
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::HumidityTemperatureSensor::msg_t* msg)
{
  uint8_t old_precision = outs.precision(2);
  uint8_t old_width = outs.width(-5);
  outs << msg->humidity << PSTR(" %, ")
       << msg->temperature << PSTR(" C");
  outs.width(old_width);
  outs.precision(old_precision);
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::RealTimeClock::msg_t* msg)
{
  outs << time_t(msg->time);
  return (outs);
}

IOStream&
operator<<(IOStream& outs, Domotica::Accelerometer::msg_t* msg)
{
  uint8_t old_precision = outs.precision(3);
  uint8_t old_width = outs.width(-4);
  outs << '<' << msg->x << ',' << msg->y << ',' << msg->z << '>';
  outs.width(old_width);
  outs.precision(old_precision);
  if (msg->source & _BV(Domotica::Accelerometer::FREE_FALL))
    outs << PSTR(", free fall");
  if (msg->source & _BV(Domotica::Accelerometer::INACT))
    outs << PSTR(", inactivity");
  if (msg->source & _BV(Domotica::Accelerometer::ACT))
    outs << PSTR(", activity");
  if (msg->source & _BV(Domotica::Accelerometer::DOUBLE_TAP))
    outs << PSTR(", double tap");
  if (msg->source & _BV(Domotica::Accelerometer::SINGLE_TAP))
    outs << PSTR(", single tap");
  return (outs);
}
