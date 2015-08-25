/**
 * @file Domotica.hh
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

#ifndef COSA_DOMOTICA_HH
#define COSA_DOMOTICA_HH

#include "Cosa/Types.h"
#include "Cosa/Time.hh"
#include "Cosa/IOStream.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/AnalogPin.hh"

/**
 * Default Domotica network identity. The full node address is
 * [NETWORK].[DEVICE]. A sensor address is [NETWORK].[DEVICE].[PORT].[ID].
 * Typically the NETWORK is fixed. User interfaces will need to map
 * [DEVICE].[PORT].[ID] to a human readable form.
 */
#ifndef NETWORK
#define NETWORK 0xD1CA
#endif

namespace Domotica {

  /**
   * Start Domotica with given wireless device. Returns true(1) if
   * successful otherwise false(0).
   * @param[in] dev device.
   * @param[in] config configuration (default NULL).
   * @return bool.
   */
  bool begin(Wireless::Driver* rf, void* config = NULL);

  /**
   * Sleep in power down mode for given number of seconds.
   * @param[in] s seconds (Default 5 seconds).
   */
  void sleep(uint16_t s = 5);

  /**
   * Await external interrupt pin. Sleep in power down mode until the
   * pin is low.
   * @param[in] pin external interrupt pin (default EXT0).
   */
  void await(Board::ExternalInterruptPin pin = Board::EXT0);

  /**
   * Print local address of sensor; DEVICE.ID.
   * @param[in] outs output stream.
   * @param[in] device address.
   * @param[in] port sensor type.
   * @param[in] id identity.
   */
  void print(IOStream& outs, uint8_t device, uint8_t port, uint8_t id);

  /**
   * Print full address of sensor; NETWORK.DEVICE.ID.
   * @param[in] outs output stream.
   * @param[in] network address.
   * @param[in] device address.
   * @param[in] port sensor type.
   * @param[in] id identity.
   */
  void print(IOStream& outs, uint16_t network, uint8_t device, uint8_t port, uint8_t id);

  /**
   * Domotic device message header.
   */
  struct header_t {
    /**
     * Set local source identity, sequence number and sample battery
     * level.
     * @param[in] nr message sequence number.
     * @param[in] id source identity.
     */
    void set(uint8_t &nr, uint8_t id)
    {
      this->id = id;
      this->nr = nr++;
      this->battery = AnalogPin::bandgap();
    }

    /** Local sensor/device identity. */
    uint8_t id;

    /** Message sequence number. */
    uint8_t nr;

    /** Battery level (mV). */
    uint16_t battery;
  };

  /** Message types. */
  enum {
    INFO_STRING_MSG = 0,
    DIGITAL_PIN_MSG = 1,
    ANALOG_PIN_MSG = 2,
    DIGITAL_PINS_MSG = 3,
    TEMPERATURE_SENSOR_MSG = 4,
    HUMIDITY_TEMPERATURE_SENSOR_MSG = 5,
    REALTIME_CLOCK_MSG = 6
  };

  /** Message payload size (26 bytes). */
  static const size_t PAYLOAD_MAX = 30 - sizeof(header_t);

  /** Payload Message (26 bytes max). */
  struct msg_t : header_t {
    uint8_t payload[PAYLOAD_MAX];
  };

  /** Information String Message (26 bytes max). */
  namespace InfoString {
    static const size_t MAX = PAYLOAD_MAX;
    struct msg_t : header_t {
      char info[MAX];		// Null terminated string.
    };
  };

  /** Digital Pin Sample Message (5 bytes). */
  namespace DigitalPin {
    struct msg_t : header_t {
      bool value;	       // Sample value; on(1)/off(0).
    };
  };

  /** Digital Pins Sample Message (8 bytes). */
  namespace DigitalPins {
    static const uint8_t MAX = 32;
    struct msg_t : header_t {
      uint32_t value;	       // Sample value; bit[0..ID-1]
    };
  };

  /** Analog Pin Sample Message (6 bytes). */
  namespace AnalogPin {
    struct msg_t : header_t {
      uint16_t value;	       // Sample value (0..1023).
    };
  };

  /** Digital Temperature Sensor Message (8 bytes). */
  namespace TemperatureSensor {
    struct msg_t : header_t {
      float32_t temperature;     // Temperature C.
    };
  };

  /** Digital Humidity and Temperature Sensor Message (12 bytes). */
  namespace HumidityTemperatureSensor {
    struct msg_t : header_t {
      float32_t humidity;	// Humidity RH%.
      float32_t temperature;	// Temperature C.
    };
  };

  /** Real-Time Clock Message (8 bytes). */
  namespace RealTimeClock {
    struct msg_t : header_t {
      clock_t time;		// Seconds since epoch.
    };
  };
};

/**
 * Print header to given output stream.
 * @param[in] outs output stream.
 * @param[in] header to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::header_t* header);

/**
 * Print message payload in hex format to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::msg_t* msg);

/**
 * Print information message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::InfoString::msg_t* msg);

/**
 * Print digital pin sample message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::DigitalPin::msg_t* msg);

/**
 * Print digital pins sample message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::DigitalPins::msg_t* msg);

/**
 * Print analog pin sample message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::AnalogPin::msg_t* msg);

/**
 * Print temperature sensor sample message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::TemperatureSensor::msg_t* msg);

/**
 * Print humidity and temperature sensor sample message to given
 * output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::HumidityTemperatureSensor::msg_t* msg);

/**
 * Print real-time clock message to given output stream.
 * @param[in] outs output stream.
 * @param[in] msg to print.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, Domotica::RealTimeClock::msg_t* msg);
#endif
