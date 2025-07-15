#include "flarm_message.h"
#include "flarm/flarm2024packet.hpp"
#include "flarm_frequencies.h"
#include "flarm_zones.h"
#include "instruments/gps.h"

FlarmMessage flarmMessage;  // Global instance of FlarmMessage

uint32_t FlarmMessage::sendFlarmMessage(etl::random_xorshift& random,
                                        const FANET::Address& srcAddress, SX1262& radio) {
  // Implementation of sending a FLARM message

  // Check if our FLARM zone is set, or, it has been
  // 10 minutes since the last time our zone was checked.
  // This is important in case we travel between zones.
  if (lastZoneCheckMs == 0 || millis() - lastZoneCheckMs > 10 * 60 * 1000) {
    if (!gps.location.isValid()) {
      // If the GPS location is not valid, we cannot determine the zone
      return 1000;  // try again in 1 second
    }

    zone = FlarmZone::zoneFor(gps.location.lat(), gps.location.lng());
    lastZoneCheckMs = millis();
  }

  // FANET messages can only be sent in very specific time slots.
  // Check if we are in a valid time slot to send a FLARM message.
  // Get the current accurate GPS synced time
  auto msPastSecond = gps.msPastSecond();
  const auto slot = FlarmCountryRegulations::getSlot(zone);
  const auto sendFrequency = FlarmCountryRegulations::getFrequency(slot, msPastSecond);

  if (!sendFrequency) {
    // Not in a valid time slot to send a FLARM message
    // Figure out when to try again
    Serial.println("Not in a valid time slot to send FLARM message");
    return FlarmCountryRegulations::nextRandomTimeOffset(slot, msPastSecond, random);
  }

  // Prepare the FLARM packet
  Flarm2024Packet packet;
  packet.aircraftId(srcAddress.asUint());
  packet.messageType(2);  //  2 new protocol, 0 old protocol, 1 other messages
  packet.addressType(FlarmMessage::FLARM_ADDRESS_TYPE_FLARM);
  packet.stealth(false);  // Not stealth mode
  packet.noTrack(false);  // Not no-track mode

  // Epoch seconds
  tm cal;
  gps.getUtcDateTime(cal);
  auto epochSeconds = mktime(&cal);
  packet.epochSeconds(epochSeconds);
  packet.aircraftType((uint8_t)FlarmAircraftType::PARA_GLIDER);
  packet.altitude(gps.altitude.meters());
  packet.setPosition(gps.location.lat(), gps.location.lng());
  packet.turnRate(0);  // No turn rate
  packet.groundSpeed(gps.speed.mps());
  packet.verticalSpeed(0);                                      // No vertical speed... TODO:  This
  packet.groundTrack(gps.course.deg());                         // Ground track
  packet.movementStatus((uint8_t)FlarmMovementStatus::FLYING);  // 2=Flying, 1=On ground, 0=Unknown

  // Tune our radio to the correct frequency
  radio.beginFSK(sendFrequency,
                 100.0f,  // FSK bit rate in kbps
                 5.0f,    // Frequency deviation from carrier frequency in kHz
                 slot.frequency.bandwidth, slot.frequency.powerdBm,
                 16  // FSK preamble length in bits
  );
  radio.setPreambleLength(16);  // Set preamble length
  uint8_t syncWord[8] = {0x55, 0x99, 0xA5, 0xA9, 0x55, 0x66, 0x65, 0x96};
  radio.setSyncWord(syncWord, 8);
  radio.fixedPacketLengthMode(26 * 2);  // NOT sure why *2 is needed here.

  // Write the packet to the radio buffer
  uint32_t writeBuffer[Flarm2024Packet::TOTAL_LENGTH_WORDS];
  etl::span<uint32_t> writeSpan(writeBuffer, Flarm2024Packet::TOTAL_LENGTH_WORDS);
  packet.writeToBuffer(epochSeconds, writeSpan);

  // Transmit the packet
  int16_t status = radio.transmit((uint8_t*)writeBuffer, Flarm2024Packet::TOTAL_LENGTH);
  if (status != RADIOLIB_ERR_NONE) {
    Serial.println("Failed to transmit FLARM message: " + String(status));
  }

  Serial.println("FLARM message sent");

  // Return when to send the next message at 4 second intervals
  return 4000 + FlarmCountryRegulations::nextRandomTimeOffset(slot, msPastSecond, random);
}