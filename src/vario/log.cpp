

#include "log.h"
#include "baro.h"
#include "gps.h"
#include "IMU.h"


uint32_t flightTimerSec = 0;
bool flightTimerRunning = 0;

// values for logbook entries
  int32_t log_alt = 0;
  int32_t log_alt_start = 0;
  int32_t log_alt_end = 0;
  int32_t log_alt_max = 0;
  int32_t log_alt_min = 0;
  int32_t log_alt_above_launch = 0;
  int32_t log_alt_above_launch_max = 0;

  int16_t log_climb = 0;
  int16_t log_climb_max = 0;
  int16_t log_climb_min = 0;

  int32_t log_temp = 0;
  int32_t log_temp_max = 0;
  int32_t log_temp_min = 0;





// update function to run every second
void logging_udpate() {
  if (flightTimerRunning) {
    flightTimerSec++;
  }

  //check altitude values for log
  log_alt = baro_getAlt();
  log_alt_above_launch = baro_getAltAboveLaunch();
  if (log_alt > log_alt_max) {
    log_alt_max = log_alt;
    if (log_alt_above_launch > log_alt_above_launch_max) log_alt_above_launch_max = log_alt_above_launch; // we only need to check for max above-launch values if we're also setting a new altitude max.
  } else if (log_alt < log_alt_min) {
    log_alt_min = log_alt;
  } 

  //check climb values for log
  log_climb = baro_getClimbRate();
  if (log_climb > log_climb_max) {
    log_climb_max = log_climb;
  } else if (log_climb < log_climb_min) {
    log_climb_min = log_climb;
  } 

  // check temperature values
  log_temp = baro_getTemp();
  if (log_temp > log_temp_max) {
    log_temp_max = log_temp;
  } else if (log_temp < log_temp_min) {
    log_temp_min = log_temp;
  } 

  // save log values in SD card
  // TODO
}


void flightTimer_start() {
  flightTimerRunning = 1;

  //starting values
  baro_resetLaunchAlt();
  log_alt_start = baro_getAltAtLaunch();
  
}

void flightTimer_stop() {
  flightTimerRunning = 0;

  //ending values
  log_alt_end = baro_getAlt();
}

void flightTimer_reset() {
  flightTimer_stop();
  flightTimerSec = 0;
}
