/**
 * @file DomoticaADXL345.ino
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
 * Domotica demonstation sketch; Interrupt driven accelerometer
 * measurement and message send.
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
 *                           GY-291
 *                       +------------+
 * (GND)---------------1-|GND         |
 * (VCC)---------------2-|VCC         |
 *                     3-|CS          |
 * (D2/EXT0)-----------4-|A-INT1      |
 *                     5-|A-INT2      |
 *                     6-|SDO         |
 * (A4/SDA)------------7-|SDA         |
 * (A5/SCL)------------8-|SCL         |
 *                       +------------+
 *
 * @endcode
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include <Domotica.h>
#include <Domotica/RF433.h>

// Default device address
#define DEVICE 0x90

// RF433 includes; Virtual Wire Wireless Interface and Huffman(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI rf(NETWORK, DEVICE, SPEED, RX, TX, &codec);

// Sketch includes
#include "Cosa/RTC.hh"
#include "Cosa/OutputPin.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/ExternalInterrupt.hh"
#include <ADXL345.h>

// Digital accelerometer with alternative address
ADXL345 accelerometer(1);

// Flash led during transmission
OutputPin led(Board::LED, 0);

class InterruptPin : public ExternalInterrupt {
public:
  InterruptPin(Board::ExternalInterruptPin pin) :
    ExternalInterrupt(pin, ON_FALLING_MODE, true)
  {}

  virtual void on_interrupt(uint16_t arg)
  {
    UNUSED(arg);
    disable();
  }
} intr(Board::EXT0);

void setup()
{
  Domotica::begin(&rf);
  accelerometer.begin();
}

void loop()
{
  static uint8_t nr = 0;

  Power::all_disable();
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
  Watchdog::end();
  RTC::end();
  intr.enable();
  while (intr.is_set()) yield();
  Power::set(mode);
  Watchdog::begin();
  RTC::begin();
  Power::all_enable();
  ::AnalogPin::powerup();

  uint8_t source = accelerometer.is_activity();
  if (source != 0) {
    Domotica::Accelerometer::msg_t msg;
    ADXL345::sample_t value;
    msg.set(nr, 0);
    msg.source = source;
    accelerometer.sample(value);
    msg.x = value.x * 0.004;
    msg.y = value.y * 0.004;
    msg.z = value.z * 0.004;
    led.on();
    rf.powerup();
    rf.broadcast(Domotica::ACCELEROMETER_MSG, &msg, sizeof(msg));
    rf.powerdown();
    led.off();
  }
}
