/**
 * @file DomoticaMonitor.ino
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
 * Domotica demonstration sketch; Recieve messages and print to LCD.
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
#define DEVICE 0x02

// RF433 includes; Virtual Wire Wireless Interface and Hamming(7,4) codec
#include <VWI.h>
#include <HammingCodec_7_4.h>

HammingCodec_7_4 codec;
VWI::Receiver rx(RX, &codec);
VWI rf(NETWORK, DEVICE, SPEED, &rx);

// Sketch includes
#include "Cosa/Time.hh"
#include "Cosa/IOStream.hh"

// Select port type to use with the LCD device driver.
// LCD and communication port
#include <HD44780.h>

// HD44780 driver built-in adapters
// HD44780::Port4b port;
// HD44780::SR3W port;
// HD44780::SR3WSPI port;
// HD44780::SR4W port;

// PCF8574 I2C expander io port based adapters
#include <PCF8574.h>
// #include <MJKDZ_LCD_Module.h>
// MJKDZ_LCD_Module port;
// MJKDZ_LCD_Module port(0);
// #include <GY_IICLCD.h>
// GY_IICLCD port;
#include <DFRobot_IIC_LCD_Module.h>
DFRobot_IIC_LCD_Module port;
// #include <SainSmart_LCD2004.h>
// SainSmart_LCD2004 port;

// MCP23008 I2C expander io port based adapters
// #include <MCP23008.h>
// #include <Adafruit_I2C_LCD_Backpack.h>
// Adafruit_I2C_LCD_Backpack port;

// HD44780 lcd(&port, 20, 4);
// HD44780 lcd(&port, 16, 4);
HD44780 lcd(&port);
IOStream cout(&lcd);

void setup()
{
  Domotica::begin(&rf);
  time_t::epoch_year(2015);
  lcd.begin();
  cout << PSTR("DomoticaMonitor: started");
  rf.powerup();
}

void loop()
{
  const uint32_t TIMEOUT = 10000L;
  Domotica::msg_t msg;
  uint8_t src;
  uint8_t port;

  while (rf.recv(src, port, &msg, sizeof(msg), TIMEOUT) < 0);
  cout << clear;
  if (msg.battery < 3500) cout << '*';
  Domotica::print(cout, src, port, &msg);
}
