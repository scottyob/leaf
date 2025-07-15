#pragma once

#include <etl/array.h>
#include <etl/random.h>
#include <cstdint>
#include "flarm_zones.h"

class FlarmCountryRegulations {
 public:
  static constexpr uint32_t SLOT_MS = 200;

  struct Frequency {
    uint32_t baseFrequency;
    uint32_t channelSeparation;
    uint8_t channels;
    int8_t powerdBm;
    uint16_t bandwidth;
  };

  enum class Channel : uint8_t {
    // Europe has 2 channels channel 0 = 868.2MHz, channel 1 = 868.4MHz. Frequency is finaly decided
    // by the frequency table per area
    NOP,
    CH0,
    CH1
  };

  // From https://github.com/VirusPilot/esp32-ogn-tracker
  static constexpr Frequency Europe{868'200'000, 200'000, 02, 14, 250};
  static constexpr Frequency NorthAmerica{902'200'000, 400'000, 65, 30, 0};
  static constexpr Frequency NewZealand{869'250'000, 200'000, 01, 10, 0};
  static constexpr Frequency Australia{917'000'000, 400'000, 24, 30, 0};
  static constexpr Frequency Israel{916'200'000, 200'000, 01, 22, 0};
  static constexpr Frequency SouthAmerica{917'000'000, 400'000, 24, 30, 0};
  //    static constexpr Frequency PawEurope{869'525'000, 000'000, 00, 14, 100};
  //    static constexpr Frequency EuropeFanet{868'200'000, 000'000, 01, 14, 250};

  // clang-format off
    // First byte of the syncWord is the preamble for TX
    //                                                                mode                   dataSource  packetLength txPreambleLength codingRate syncLength;
    // static constexpr Radio::ProtocolConfig PROTOCOL_FLARM{2, GATAS::Modulation::GFSK, GATAS::DataSource::FLARM,  26, 16, 8, {0x55, 0x99, 0xA5, 0xA9, 0x55, 0x66, 0x65, 0x96}};       // 0 FLARM 0 airtime 6ms    

    struct ProtocolTimeSlot
    {
        uint8_t ptsId;              // Unique ID for each ProtocolTiming. Use internally to oprmise configurrion of the transceiver
        FlarmZone zone;
        Frequency frequency;
        etl::array<Channel, 5> timing;
        uint16_t txMinTime;
        uint16_t txMaxTime;
        uint8_t waitAfterCatStart;
        uint8_t waitAfterCatEnd;
    };

    static constexpr etl::array<const ProtocolTimeSlot, 9> protocolTimings{
      ProtocolTimeSlot{1, FlarmZone::ZONE0, Europe, {Channel::NOP, Channel::NOP, Channel::NOP, Channel::NOP, Channel::NOP},  000, 0000, 00, 000},
      ProtocolTimeSlot{2, FlarmZone::ZONE1, Europe, {Channel::CH1, Channel::NOP, Channel::CH0, Channel::CH0, Channel::CH1},  600, 1400, 15, 150},
      ProtocolTimeSlot{3, FlarmZone::ZONE5, Israel, {Channel::CH1, Channel::NOP, Channel::CH0, Channel::CH0, Channel::CH1},  600, 1400, 15, 150},

      // New Zeland not tested
      ProtocolTimeSlot{7, FlarmZone::ZONE3, NewZealand, {Channel::CH1, Channel::NOP, Channel::CH0, Channel::CH0, Channel::CH1},  600, 1400, 15, 150},

      // TODO:  Figure out US, Australia... Pretty much everywhere else.
    };
 private:
  FlarmCountryRegulations() = delete;

 public:
  /**
   * @brief get the correct slot for the Zone. This decides everything on what
   * frequency to use, timings etc..
   *
   * @param zone
   * @param dataSource
   * @return const CountryRegulations::ProtocolTimeSlot& Reference to the slot configuration
   */
  static const ProtocolTimeSlot& getSlot(const FlarmZone zone);

  /**
   * @brief Decide on the frequency from the configuration and the channel
   * Note: Only CH0 and CH1 is supported, NOP is not a valid channel
   *
   * @param frequency
   * @param channel
   * @return uint32_t frequency in Hz
   */
  static uint32_t getFrequency(const ProtocolTimeSlot& timeSlot, const unsigned short msPastSecond = 0);

  /**
   * @brief Get the next random time for this protocol that falls within the timeslot
   *
   * @param pts 
   * @param msPastSecond the number of milliseconds past the current second
   * @return uint32_t
   */
  static uint32_t nextRandomTimeOffset(const ProtocolTimeSlot& pts, unsigned short msPastSecond, etl::random_xorshift& random);
};
