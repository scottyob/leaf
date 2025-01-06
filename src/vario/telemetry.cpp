#include "telemetry.h"

#include <SD_MMC.h>

#include "Arduino.h"
#include "gps.h"
#include "settings.h"
#include "log.h"

bool Telemetry_t::begin() {
  auto date = gps_getIso8601Date();
  if (date.isEmpty()) return false;
  String fileName = "/Data/TestData_";
  fileName += date;
  fileName += "_";

  // Add minues and seconds to the filename
  int32_t timeInMinutes =
      (gps.time.hour() * 60 + gps.time.minute() + 24 * 60 + TIME_ZONE) % (24 * 60);
  uint8_t timeHours = timeInMinutes / 60;
  uint8_t timeMinutes = timeInMinutes % 60;
  fileName += String(timeHours / 10) + String(timeHours % 10) + String(timeMinutes / 10) +
              String(timeMinutes % 10);

  fileName += ".csv";

  // Open the file to write the telemetry
  file = SD_MMC.open(fileName, "w", true);
  if (!file) {
    return false;
  }

  // Write the CSV header
  file.println("millis,sensor, value (lat), lng, alt m, speed mps, heading deg");
  return true;
}

void Telemetry_t::end() { file.close(); }

void Telemetry_t::writeText(const String text) {
  // Only write telemetry if a flight has started
  if(!flightTimer_isRunning())
    return;
  
  // Try to start the telemetry file if not started
  if (!file) {
    begin();
    if (!file) return;
  }
  file.println((String)millis() + "," + text);
}

Telemetry_t Telemetry;