#ifndef log_h
#define log_h

#include <Arduino.h>


// Update function to run every second
  void log_init(void);
  void log_update(void);

// Flight Timer functions
  void flightTimer_start(void);
  void flightTimer_stop(void);
  void flightTimer_toggle(void);
  void flightTimer_reset(void);
  bool flightTimer_isRunning(void);

  void flightTimer_updateStrings(void);
  char * flightTimer_getString(bool shortString);
  uint32_t flightTimer_getTime(void);













#endif
