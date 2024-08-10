#ifndef log_h
#define log_h

#include <Arduino.h>

// Auto Start/Stop Timer Thresholds
  #define AUTO_START_MIN_SPEED  5     // mph minimum before timer will auto-start
  #define AUTO_START_MIN_SEC    3     // seconds above the min speed to auto-start
  #define AUTO_START_MIN_ALT    500   // cm altitude change for timer auto-start

  #define AUTO_STOP_MAX_SPEED   3     // mph max -- must be below this speed for timer to auto-stop
  #define AUTO_STOP_MAX_ACCEL   10    // Max accelerometer signal 
  #define AUTO_STOP_MAX_ALT    500   // cm altitude change for timer auto-stop
  #define AUTO_STOP_MIN_SEC     10    // seconds of low speed / low accel for timer to auto-stop
  
  

// Main Log functions 
  void log_init(void);    // set up log at vario start
  void log_update(void);  // Update function to run every second

// Flight Timer Auto Start/Stop check functions
  bool flightTimer_autoStart(void);
  bool flightTimer_autoStop(void);


// Flight Timer functions
  void flightTimer_start(void);
  void flightTimer_stop(void);
  void flightTimer_toggle(void);
  void flightTimer_reset(void);
  bool flightTimer_isRunning(void);

  void flightTimer_updateStrings(void);
  char * flightTimer_getString(bool shortString);
  uint32_t flightTimer_getTime(void);
//

// Log Files
  String log_createFileName(void);
  String log_getKMLCoordinates(void);
  void log_checkMinMaxValues(void);
  bool flightTimer_autoStop(void);
  bool flightTimer_autoStart(void);




#endif
