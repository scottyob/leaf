#pragma once
#include <modules/SX126x/SX1262.h>
#include <fanet/tracking.hpp>

enum class eFlarmAircraftType {
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

/// @brief Sends a FANET tracking payload using the Flarm protocol.
/// @param payload FANET payload to send
void sendUsingFlarm(FANET::TrackingPayload payload, uint32_t fanetAddress, SX1262* radio);