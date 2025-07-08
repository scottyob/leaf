#include "flarm_radio.h"
#include <instruments/gps.h>
#include "../../libraries/FLARM/include/flarm/flarm2024packet.hpp"

void sendUsingFlarm(FANET::TrackingPayload payload, uint32_t fanetAddress, SX1262* radio) {
  // Build a Flarm tracking payload

  Flarm2024Packet flarmPacket;

  eFlarmAircraftType aircraftType = eFlarmAircraftType::UNKNOWN;
  switch (payload.aircraftType()) {
    case FANET::TrackingPayload::AircraftType::GLIDER:
      aircraftType = eFlarmAircraftType::GLIDER_MOTOR_GLIDER;
      break;
    case FANET::TrackingPayload::AircraftType::PARAGLIDER:
      aircraftType = eFlarmAircraftType::PARA_GLIDER;
      break;
    case FANET::TrackingPayload::AircraftType::HANGLIDER:
      aircraftType = eFlarmAircraftType::HANG_GLIDER;
      break;
    case FANET::TrackingPayload::AircraftType::BALLOON:
      aircraftType = eFlarmAircraftType::BALLOON;
      break;
    case FANET::TrackingPayload::AircraftType::POWERED_AIRCRAFT:
      aircraftType = eFlarmAircraftType::AIRCRAFT_RECIPROCATING_ENGINE;
      break;
    case FANET::TrackingPayload::AircraftType::HELICOPTER:
      aircraftType = eFlarmAircraftType::HELICOPTER_ROTORCRAFT;
      break;
    case FANET::TrackingPayload::AircraftType::UAV:
      aircraftType = eFlarmAircraftType::UAV;
      break;
    default:
      aircraftType = eFlarmAircraftType::UNKNOWN;
  }

  // Address type 0=Stateless Random, 1=Official ICAO, 2=stableFlarm
  flarmPacket.addressType(2);
  flarmPacket.aircraftId(fanetAddress);
  flarmPacket.messageType(2);  // 2 is new protocol??

  flarmPacket.setPosition(payload.latitude(), payload.longitude());
  flarmPacket.altitude(payload.altitude());
  flarmPacket.groundSpeed(payload.speed());
  flarmPacket.groundTrack(payload.groundTrack());
  flarmPacket.verticalSpeed(payload.climbRate());
  flarmPacket.aircraftType(static_cast<uint8_t>(aircraftType));

  tm cal;
  gps.getUtcDateTime(cal);
  auto epochSeconds = mktime(&cal);  // Convert to time_t for epoch seconds
  flarmPacket.epochSeconds(epochSeconds);

  flarmPacket.noTrack(!payload.tracking());

  // Write this to a buffer ready to send
  etl::array<uint32_t, Flarm2024Packet::TOTAL_LENGTH_WORDS> buffer;
  etl::span<uint32_t, Flarm2024Packet::TOTAL_LENGTH_WORDS> bufferSpan(buffer);
  flarmPacket.writeToBuffer(epochSeconds, bufferSpan);

  // Send the packet out over the wire
  int state = radio->beginFSK();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Flarm radio begin failed: ");
    Serial.println(state);
    return;
  }

  // Switch back to LoRa mode
  radio->begin();
}