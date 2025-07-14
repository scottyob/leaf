#pragma once

#include <etl/enum_type.h>

/// @brief Area of the world where a protocol is used.
///        Used to determine frequencies and protocols for a given region.
///        Should be called sparingly or when you think you may have changed regions.
struct FlarmZone {
  enum enum_type {
    ZONE0,  // ZONE0 is unknown and no transmission will take place
    ZONE1,  // Zone 1: Europe, Africa, Russia, China (30W to 110E, excl. zone 5) All protocols MUST
            // be part of ZONE1. see : RadioTuner::addDataSourceToTasks()
    ZONE2,  // Zone 2: North America (west of 30W, north of 10N)
    ZONE3,  // Zone 3: New Zealand (east of 160E)
    ZONE4,  // Zone 4: Australia (110E to 160E)
    ZONE5,  // Zone 5: Israel (34E to 54E and 29.25N to 33.5N
    ZONE6   // Zone 6: South America (west of 30W, south of 10N)
  };

  ETL_DECLARE_ENUM_TYPE(FlarmZone, int)
  ETL_ENUM_TYPE(ZONE0, "Zone0")
  ETL_ENUM_TYPE(ZONE1, "Zone1")
  ETL_ENUM_TYPE(ZONE2, "Zone2")
  ETL_ENUM_TYPE(ZONE3, "Zone3")
  ETL_ENUM_TYPE(ZONE4, "Zone4")
  ETL_ENUM_TYPE(ZONE5, "Zone5")
  ETL_ENUM_TYPE(ZONE6, "Zone6")
  ETL_END_ENUM_TYPE

  static FlarmZone zoneFor(float lat, float lon) {
    if (34.0f <= lon && lon <= 54.0f && 29.25f <= lat && lat <= 33.5f) {
      return ZONE5;  // Zone 5: Israel (34E to 54E and 29.25N to 33.5N)
    } else if (-30.0f <= lon && lon <= 110.0f) {
      return ZONE1;  // Zone 1: Europe, Africa, Russia, China (30W to 110E, excl. zone 5)
    } else if (lon < -30.0f && 10.0f < lat) {
      return ZONE2;  // Zone 2: North America (west of 30W, north of 10N)
    } else if (160.0f < lon) {
      return ZONE3;  // Zone 3: New Zealand (east of 160E)
    } else if (110.0f <= lon && lon <= 160.0f) {
      return ZONE4;  // Zone 4: Australia (110E to 160E)
    } else if (lon < -30.0f && lat < 10.0f) {
      return ZONE6;  // Zone 6: South America (west of 30W, south of 10N)
    }
    return ZONE0;  // not defined
  }
};  // Struct FanetZone
