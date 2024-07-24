

#include "log.h"
#include "baro.h"
#include "gps.h"
#include "IMU.h"
#include "speaker.h"


uint32_t flightTimerSec = 0;
bool flightTimerRunning = 1;
bool flightTimerResetting = 0;
char flightTimerString[] = "0:00:00";    // H:MM:SS -> 7 characters max for string.  If it hits 10 hours, will re-format to _HH:MM_
char flightTimerStringShort[] = "00:00"; // MM:SS -> 5 characters max for string.  If it hits 1 hour, will change to HH:MM

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


void log_init() {

}


// update function to run every second
void log_update() {
  if (flightTimerRunning) {
    flightTimerSec += 1;
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

bool flightTimer_isRunning() {
  return flightTimerRunning;
}

void flightTimer_start() {
  speaker_playSound(fx_enter);
  flightTimerRunning = 1;

  //starting values
  baro_resetLaunchAlt();
  log_alt_start = baro_getAltAtLaunch();  
}

void flightTimer_stop() {
  flightTimerRunning = 0;

  //ending values
  log_alt_end = baro_getAlt();
  if (!flightTimerResetting) speaker_playSound(fx_cancel);    // only play sound if not in the process of resetting (the resetting sound will play from the reset function)
}

void flightTimer_toggle() {  
  if (flightTimerRunning) flightTimer_stop();
  else flightTimer_start();
}

void flightTimer_reset() {
  flightTimerResetting = true;
  if (flightTimerRunning) flightTimer_stop();
  speaker_playSound(fx_exit);
  flightTimerSec = 0;  
  flightTimerResetting = false;
}

uint32_t flightTimer_getTime() {
  return flightTimerSec;
}

char * flightTimer_getString(bool shortString) {
  flightTimer_updateStrings();
  if (shortString) return flightTimerStringShort;
  else return flightTimerString;
}

void flightTimer_updateStrings() {
  // char * flightTimerString = "0:00:00";    // H:MM:SS -> 7 characters max for string.  If it hits 10 hours, will re-format to _HH:MM_
  // char * flightTimerStringShort = "00:00"; // MM:SS -> 5 characters max for string.  If it hits 1 hour, will change to HH:MM

  uint8_t hours = flightTimerSec / 3600;
  uint8_t mins  = (flightTimerSec / 60) % 60;
  uint8_t secs  = flightTimerSec % 60;

  uint8_t position = 0;

  // update flightTimerString    
  if (hours > 9) {
    if (hours > 99) 
    //      "000:00 "  HHH:MM_
      flightTimerString[position] = '0' + hours/100;
    else            
    //      " 00:00 "  _HH:MM_
      flightTimerString[position] = ' ';
      flightTimerString[++position] = '0' + hours/10;
      flightTimerString[++position] = '0' + hours % 10;
      flightTimerString[++position] = ':';
      flightTimerString[++position] = '0' + mins/10;
      flightTimerString[++position] = '0' + mins % 10;
      flightTimerString[++position] = ' ';
  } else if (hours > 0) {
    //      "0:00:00"  H:MM:SS
      flightTimerString[position] = '0' + hours;
      flightTimerString[++position] = ':';
      flightTimerString[++position] = '0' + mins/10;
      flightTimerString[++position] = '0' + mins % 10;
      flightTimerString[++position] = ':';
      flightTimerString[++position] = '0' + secs/10;
      flightTimerString[++position] = '0' + secs % 10;
  } else {
      flightTimerString[position++] = ' ';
    if (mins > 9)
    //      " 00:00 "  _MM:SS_
      flightTimerString[position] = '0' + mins/10;
    else 
    //      "  0:00 "  __M:SS_
      flightTimerString[position] = ' ';
      
      flightTimerString[++position] = '0' + mins % 10;
      flightTimerString[++position] = ':';
      flightTimerString[++position] = '0' + secs/10;
      flightTimerString[++position] = '0' + secs % 10;
      flightTimerString[++position] = ' ';
  }

    position = 0;

    // update flightTimerStringShort
      // char * flightTimerStringShort = "00:00"; // MM:SS -> 5 characters max for string.  If it hits 1 hour, will change to HH:MM
    if (hours > 0) {
      if (hours > 9) flightTimerStringShort[position] = '0' + hours/10;
      else flightTimerStringShort[position] = ' ';
      flightTimerStringShort[++position] = '0' + hours % 10;
      flightTimerStringShort[++position] = ':';
      flightTimerStringShort[++position] = '0' + mins/10;
      flightTimerStringShort[++position] = '0' + mins % 10;
    } else {
      if (mins > 9) flightTimerStringShort[position] = '0' + mins/10;
      else flightTimerStringShort[position] = ' ';
      flightTimerStringShort[++position] = '0' + mins % 10;
      flightTimerStringShort[++position] = ':';
      flightTimerStringShort[++position] = '0' + secs/10;
      flightTimerStringShort[++position] = '0' + secs % 10;
    }
    
       
}




















