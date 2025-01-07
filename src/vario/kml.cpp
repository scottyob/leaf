#include "kml.h"

#include "gps.h"
#include "settings.h"
#include "string_utils.h"

const String Kml::desiredFileName() const {
  String fileTitle = "FlightTrack";
  String fileDate = gps_getIso8601Date();
  int32_t timeInMinutes =
      (gps.time.hour() * 60 + gps.time.minute() + 24 * 60 + TIME_ZONE) % (24 * 60);
  uint8_t timeHours = timeInMinutes / 60;
  uint8_t timeMinutes = timeInMinutes % 60;
  String fileTime = String(timeHours / 10) + String(timeHours % 10) + String(timeMinutes / 10) +
                    String(timeMinutes % 10);
  // TODO: add seconds in case two files are started within a minute

  Serial.println(timeInMinutes);
  Serial.println(timeHours);
  Serial.println(timeMinutes);
  Serial.println(timeMinutes / 10);
  Serial.println(timeMinutes % 10);

  String fileName = fileTitle + "_" + fileDate + "_" + fileTime;
  Serial.println(fileName);

  return fileName;
}

void Kml::startFlight() {
  Flight::startFlight();
  file.println(KMLtrackHeader);
}

void Kml::log(unsigned long durationSec) {
  String lonPoint = String(gps.location.lng(), 7);
  String latPoint = String(gps.location.lat(), 7);
  String altPoint = String(gps.altitude.meters(), 2);
  String logPointStr = lonPoint + "," + latPoint + "," + altPoint + "\n";

  file.println(logPointStr);
}

void Kml::end(const FlightStats stats) {
  file.println(KMLtrackFooterA);
  file.println("Flight Time: " + formatSeconds(stats.duration, false, 0));
  file.println(KMLtrackFooterB);
  file.println(stats.toString());
  file.println(KMLtrackFooterC);
  file.println(file.name());  // KML file title (in google earth) same as long file name on SDcard
  file.println(KMLtrackFooterD);
  // skipping KML file description.  Not needed and clogs up the google earth places list
  file.println(KMLtrackFooterE);
  Flight::end(stats);
}