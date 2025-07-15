#pragma once
#include <modules/SX126x/SX1262.h>
#include <cstdint>
#include "etl/random.h"
#include "fanet/address.hpp"
#include "flarm_zones.h"

class FlarmMessage {
 public:
  enum FlarmAddressType {
    FLARM_ADDRESS_TYPE_RANDOM = 0,  // Random address type
    FLARM_ADDRESS_TYPE_ICAO = 1,    // ICAO address type
    FLARM_ADDRESS_TYPE_FLARM = 2,   // FLARM address type

    // From
    // https://github.com/lyusupov/SoftRF/blame/master/software/firmware/source/SoftRF/src/protocol/radio/Legacy.h
    // Not sure how wide spread these are.
    FLARM_ADDRESS_TYPE_ANONYMOUS = 3,  // Anonymous address type (FLARM Stealth, OGN)
    FLARM_ADDRESS_TYPE_P3I = 4,        // Yeah no idea what this is
    FLARM_ADDRESS_TYPE_FANET = 5,      // FANET address type
  };

  enum class FlarmAircraftType : uint8_t {
    // From https://gitlab.bzed.at/github-mirror/GXAirCom/-/blob/solar_mount/lib/FLARM/Flarm.h
    UNKNOWN = 0,
    GLIDER_MOTOR_GLIDER = 1,
    TOW_PLANE = 2,
    HELICOPTER_ROTORCRAFT = 3,
    SKYDIVER = 4,
    DROP_PLANE_SKYDIVER = 5,
    HANG_GLIDER = 6,
    PARA_GLIDER = 7,
    AIRCRAFT_RECIPROCATING_ENGINE = 8,
    AIRCRAFT_JET_TURBO_ENGINE = 9,
    RESERVED10 = 10,
    BALLOON = 11,
    AIRSHIP = 12,
    UAV = 13,
    RESERVED14 = 14,
    STATIC_OBJECT = 15
  };

  enum class FlarmMovementStatus : uint8_t {
    UNKNOWN = 0,    // Unknown
    ON_GROUND = 1,  // On ground
    FLYING = 2,     // Flying
  };

  // Default constructor
  FlarmMessage() = default;

 private:
  // Prevent copying
  FlarmMessage(const FlarmMessage&) = delete;
  FlarmMessage& operator=(const FlarmMessage&) = delete;

  FlarmZone zone = FlarmZone::ZONE0;  // Default zone
  unsigned long lastZoneCheckMs = 0;  // Last time the zone was checked

 public:
  /// @brief Sends our location in a FLARM message.  Will block until complete.
  /// @return The offset we should send the next message in milliseconds.
  uint32_t sendFlarmMessage(etl::random_xorshift& random, const FANET::Address& srcAddress,
                            SX1262& radio);
};

extern FlarmMessage flarmMessage;  // Declare a global instance of FlarmMessage