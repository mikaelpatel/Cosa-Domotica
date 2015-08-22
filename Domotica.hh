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
#include "Cosa/IOStream.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/AnalogPin.hh"

/**
 * Default Domotica network identity.
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
   * Domotic device message header.
   */
  struct header_t {
    /**
     * Set device identity, sequence number and sample battery
     * level.
     * @param[in] nr message sequence number.
     * @param[in] id device identity.
     */
    void set(uint8_t &nr, uint8_t id)
    {
      this->id = id;
      this->nr = nr++;
      this->battery = AnalogPin::bandgap();
    }

    /** Device identity. */
    uint8_t id;

    /** Message sequence number. */
    uint8_t nr;

    /** Domotica device battery level (mV). */
    uint16_t battery;
  };

  /** Message types. */
  enum {
    RAW_PAYLOAD_MSG = 0,
    DIGITAL_PIN_MSG = 1,
    DIGITAL_PINS_MSG = 2,
    ANALOG_PIN_MSG = 3,
    DS18B20_SENSOR_MSG = 4,
    DHT_SENSOR_MSG = 5
  };

  /** Message payload size. */
  static const size_t PAYLOAD_MAX = 32 - sizeof(header_t);

  /** Message data type. */
  struct msg_t {
    header_t header;
    uint8_t payload[PAYLOAD_MAX];
  };

  /** Analog Pin Sample Message. */
  namespace AnalogPin {
    struct msg_t {
      header_t header;	       // Domotica message header.
      uint16_t value;	       // Sample value (0..1023).
    };
  };

  /** Digital Pin Sample Message. */
  namespace DigitalPin {
    struct msg_t {
      header_t header;	       // Domotica message header.
      bool value;	       // Sample value.
    };
  };

  /** Digital Pins Sample Message. */
  namespace DigitalPins {
    struct msg_t {
      header_t header;	       // Domotica message header.
      uint32_t value;	       // Sample value.
    };
  };

  /** DS18B20 Sensor Message. */
  namespace DS18B20 {
    struct msg_t {
      header_t header;	       // Domotica message header.
      int16_t temperature;     // Temperature fixpoint<12:4>.
    };
  };

  /**
   * Digital Humidity and Temperature Sensor Message.
   */
  namespace DHT {
    struct msg_t {
      header_t header;		// Domotica message header.
      int16_t humidity;		// Humidity RH% X 10.
      int16_t temperature;	// Temperature C X 10.
    };
  };
};

IOStream& operator<<(IOStream& outs, Domotica::header_t* header);
IOStream& operator<<(IOStream& outs, Domotica::msg_t* msg);
IOStream& operator<<(IOStream& outs, Domotica::DigitalPin::msg_t* msg);
IOStream& operator<<(IOStream& outs, Domotica::DigitalPins::msg_t* msg);
IOStream& operator<<(IOStream& outs, Domotica::AnalogPin::msg_t* msg);
IOStream& operator<<(IOStream& outs, Domotica::DS18B20::msg_t* msg);
IOStream& operator<<(IOStream& outs, Domotica::DHT::msg_t* msg);

#endif
