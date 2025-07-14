#include "flarm_frequencies.h"

const FlarmCountryRegulations::ProtocolTimeSlot& FlarmCountryRegulations::getSlot(
    const FlarmZone zone) {
  for (size_t i = 0; i < protocolTimings.size(); ++i) {
    const auto& slot = protocolTimings[i];
    if (slot.zone == zone) {
      return protocolTimings[i];
    }
  }
  return protocolTimings[0];  // Not found
}

uint32_t FlarmCountryRegulations::getFrequency(const Frequency& frequency, Channel channel) {
  switch (channel) {
    case Channel::CH0:
      return frequency.baseFrequency + (frequency.channelSeparation * 0);
    case Channel::CH1:
      return frequency.baseFrequency + (frequency.channelSeparation * 1);
    default:
      return frequency.baseFrequency;  // NOP
  }
}

uint32_t FlarmCountryRegulations::nextRandomTimeOffset(const ProtocolTimeSlot& pts,
                                                       unsigned short msPastSecond,
                                                       etl::random_xorshift& random) {
  constexpr auto variations = etl::make_array<int16_t>(0, -200, 200);

  constexpr uint32_t MS_IN_SECOND = 1000;

  const auto maxRandTime = pts.txMaxTime - pts.txMinTime;
  const auto baseOffset = random.range(pts.txMinTime, pts.txMaxTime);

  size_t i = 0;
  do {
    int32_t totalOffset = static_cast<int32_t>(baseOffset) + variations[i];

    uint32_t targetMs = (msPastSecond + totalOffset) % MS_IN_SECOND;
    size_t slotIndex = targetMs / SLOT_MS;

    if (pts.timing[slotIndex] != Channel::NOP && totalOffset >= pts.txMinTime &&
        totalOffset <= pts.txMaxTime) {
      return totalOffset;
    }

    ++i;
  } while (i < variations.size());

  // fallback: 1s from now
  return 1000;
}
