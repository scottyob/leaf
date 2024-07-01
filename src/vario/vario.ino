#include <Arduino.h>
#include <HardwareSerial.h>

#include "buttons.h"
#include "power.h"
#include "Leaf_SPI.h"
#include "SDcard.h"
#include "display.h"
#include "gps.h"
#include "baro.h"
#include "IMU.h"
#include "speaker.h"
#include "SDcard.h"

// Pinout for ESP32
  #define AVAIL_GPIO_0       0  // unused, broken out to header
  #define AVAIL_GPIO_21     21  // unused, broken out to header (also used as LCD backlight if desired)
  #define AVAIL_GPIO_39     39  // unused, broken out to header

// keep track of what turned us on (usb plug or user power button), so we know what to initialize and display
  uint8_t bootUpState;

// Main system event/task timer
  hw_timer_t *task_timer = NULL;
  #define TASK_TIMER_FREQ 1000  // run at 1000Hz 
  #define TASK_TIMER_LENGTH 10  // trigger the ISR every 10ms
  void onTaskTimer(void);

  // Counters for system task timer
  char counter_10ms_block = 0;
  char counter_100ms_block = 0;

// flag if the GPS serial port is done sending data for awhile
  bool gps_is_quiet = 0;

// Trackers for Task Manager Queue.  Default to tasks being needed, so they execute upon startup
  char taskman_setTasks = 1;  // the task of setting tasks -- usually set by timer-driven interrupt

  char taskman_buttons = 1;   // poll & process buttons
  char taskman_baro = 1;      // (1) preprocess on-chip ADC pressure, (2) read pressure and preprocess on-chip ADC temperature, (3) read temp and calulate true Alt, (4) filter Alt, update climb, and store values etc
  char taskman_imu = 1;       // read sensors and update values
  char taskman_gps = 1;       // process any NMEA strings and update values
  char taskman_display = 1;   // update display
  char taskman_power = 1;     // check battery, check auto-turn-off, etc
  char taskman_logging = 1;   // check auto-start, increment timers, update log file, etc


// temp testing stuff
  uint8_t display_page = 0;
  uint8_t display_do_tracker = 1;
  #define TESTING_LOOP false




/////////////////////////////////////////////////
//             SETUP              ///////////////
/////////////////////////////////////////////////
void setup() {

  // Start USB Serial Debugging Port
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Setup");

  // Initialize buttons and power first so we can check what state we're powering up in (i.e., if a button powered us on or being plugged in to USB or something else)
    buttons_init();     Serial.println("Finished buttons");
    bootUpState = power_init(); Serial.println("Finished Power");

  // Then initialize speaker so we can play sound to user confirming startup (so they know they can let go of the power button)
    speaker_init();     Serial.println("Finished Speaker");
    speaker_playSound(fx_enter);

  // Then initialize the rest of the system
    display_init();     Serial.println("Finished display");   // u8g2 library initializes SPI bus for itself, so this can come before spi_init()
    //TODO: show loading / splash Screen?
    spi_init();         Serial.println("Finished SPI");
    //GLCD_init();        Serial.println("Finished GLCD");    // test SPI object to write directly to LCD (instead of using u8g2 library -- note it IS possible to have both enabled at once)
    baro_init();        Serial.println("Finished Baro");
    imu_init();         Serial.println("Finished IMU");
    gps_init();         Serial.println("Finished GPS");    
    SDcard_init();      Serial.println("Finished SDcard");
  
  //Start Main System Timer for Interrupt Events (this will tell Main Loop to set tasks every interrupt cycle)
    task_timer = timerBegin(TASK_TIMER_FREQ);            
    timerAttachInterrupt(task_timer, &onTaskTimer); // timer, ISR call          NOTE: timerDetachInterrupt() does the opposite
    timerAlarm(task_timer, TASK_TIMER_LENGTH, true, 0);      // auto reload timer ever time we've counted a sample length

  // All done!
    Serial.println("Finished Setup");
}

// Main Timer Interrupt
void IRAM_ATTR onTaskTimer() {
  //do stuff every alarm cycle (default 10ms)
  taskman_setTasks = 1; // next time through main loop, set tasks to do!
  // wakeup();  // go back to main loop and keep processing stuff that needs doing!
  spi_checkFlag(1);
}




/////////////////////////////////////////////////
// Main Loop for Processing Tasks ///////////////
/////////////////////////////////////////////////
/*
The main loop perioritizes processesing any data in the serial buffer so as not to miss any NMEA GPS characters.  
When the serial buffer is empty, the main loop will move on to processing any other remaining tasks.

The gps serial port is considered 'quiet' if we received the last character of the last NMEA sentence.  We shouldn't expect more sentences for awhile (~1 second GPS updates)
If the serial port is quiet, and all tasks are done, then the processor will go to sleep for the remainder of the 10ms time block.
Every 10ms, driven by an interrupt timer, the system will wake up, set flags for which tasks are needed, and then return to running the main loop.

TODO: In addition to the timer-driven interrupt, we may consider also setting wake-up interrupts for the pushbuttons, the GPS 1PPS signal, and perhaps others.
*/

void loop() {
  if (TESTING_LOOP) main_loop_test();
  else main_loop_real();
}

uint32_t millisBegin;

void main_loop_real() {    
  if (taskman_setTasks) {   // check flag set by timer interrupt.  We only do this once every 10ms
    millisBegin = millis();   // save the timestamp when we enter this 10ms block... we'll need to be done within 10ms!
    Serial.println(millisBegin);
    setTasks();             // set necessary tasks for this 10ms block
    taskman_setTasks = 0;   // we don't need to do this again until the next 10ms timer interrupt
  }

  // do necessary tasks
  taskManager(); 

 
  
  // then process serial buffer for the remained of this time blockfirst process serial buffer
  if (millis() < millisBegin+8) {
    gps_is_quiet = gps_read_buffer();   // this will loop while data is available in the buffer and return when it's all emptied.
  }

  
  
  //if (gps_is_quiet) goToSleep();                 
  //else, run this loop again and keep processing the serial buffer until we're done with all the NMEA sentences this cycle

  //TODO: if gps serial buffer can fill while processor is sleeping, then we don't need to wait for serial port to be quiet
}


void setTasks(void) {    
  // increment time counters
  if(++counter_10ms_block >= 10) {
    counter_10ms_block = 0;           // every 10 periods of 10ms, go back to 0 (100ms total)
    if (++counter_100ms_block >= 10) {
      counter_100ms_block = 0;        // every 10 periods of 100ms, go back to 0 (1sec total)      
    }
  }

  // tasks to complete every 10ms
  taskman_buttons = 1;

  // set additional tasks to complete, broken down into 10ms block cycles.  (embedded if() statements allow for tasks every second, spaced out on different 100ms blocks)
  switch (counter_10ms_block) {
    case 0:
      taskman_baro = 1;  // begin updating baro every 50ms on the 0th and 5th blocks
      break;
    case 1:
      taskman_baro = 2;
      break;
    case 2:
      taskman_baro = 3;
      break;
    case 3:
      taskman_baro = 4;
      break;
    case 4:
      taskman_imu = 1;  // update accel every 100ms during the 4th block
      break;
    case 5:
      taskman_baro = 1;  // begin updating baro every 50ms on the 0th and 5th blocks
      break;
    case 6:
      taskman_baro = 2;
      break;
    case 7:
      taskman_baro = 3;
      break;
    case 8:
      taskman_baro = 4;
      break;
    case 9:
      
      // Tasks every second complete here in the 9th 10ms block.  Pick a unique 100ms block for each task to space things out
      if (counter_100ms_block == 0) taskman_gps = 1;                              // every second: gps
      if (counter_100ms_block == 1) taskman_power = 1;                            // every second: power checks      
      if (counter_100ms_block == 2) taskman_logging = 1;                          // every second: logging

      // Update LCD every half-second on the 3rd and 8th 100ms blocks
      if (counter_100ms_block == 3 || counter_100ms_block == 8) taskman_display = 1;  // every half-second: LCD update
      break;      
  }
}



// execute necessary tasks while we're awake and have things to do
void taskManager(void) {    

  if (taskman_buttons) { buttons_update(); taskman_buttons = 0; }
  if (taskman_baro)    { baro_update(taskman_baro); taskman_baro = 0; }    // update baro, using the appropriate step number
  if (taskman_imu)     { imu_update();     taskman_imu = 0; }
  if (taskman_gps)     { gps_update();     taskman_gps = 0; }
  if (taskman_power)   { power_update();   taskman_power = 0; }
//if (taskman_logging) { logging_update(); taskman_logging = 0; }
  if (taskman_display) { display_update(); taskman_display = 0; }  
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main_loop_test() {

  uint8_t button = buttons_check();
  uint8_t button_state = buttons_get_state();

  if (button_state == PRESSED) {
    if (button == LEFT || button == UP) {      
      if (display_page > 0) display_page--;
    } else if (button == RIGHT || button == CENTER || button == DOWN) {
      display_page++;
      if (display_page > 12) display_page = 12;
    }
    display_do_tracker = 1;
    Serial.print("Going to page: ");
    Serial.println(display_page);
  }

  switch (display_page) {
    case 0:
      //full_system_test();
      //gps_test_sats();
      gps_test();
      break;
    case 1:
  
      //display_test_bat_icon();      
      //display_test();      
      //power_test();
      if (display_do_tracker) {
        full_system_test();
        //display_test_real_3();
        //SDcard_test();
        //display_do_tracker = 0;
      }
      break;
    case 2:
      speaker_TEST();
      break;
    case 3:      
      //baro_test();
      break;
    case 4:
      imu_test();
      break;
    case 5:      
      display_test_big(1);
      break;
    case 6:
      display_test_big(2);      
      break;
    case 7:
      display_test_big(3);      
      break;
    case 8:
      display_test_big(4);      
      break;
    case 9:
      display_test_big(5);      
      break;
    case 12:
      speaker_playSound(fx_exit);
      delay(2000);
      power_turn_off();
      break;
  }
}

void full_system_test() {
  delay(500);
  // update baro sensor
  taskman_baro = baro_update(taskman_baro);
  if (taskman_baro == 0) taskman_baro = 1;
  
  //for (int i=0; i<300000; i++) {
    speaker_updateVarioNote(baro_getClimbRate());
  //}
  speaker_debugPrint();

  // update display
  display_thermal_page();

  // allow setting volume by sending a 0, 1, 2, 3 character over Serial  
  if (Serial.available() > 0) {
    char letter = char(Serial.read());
    while (Serial.available() > 0) {
      Serial.read();
    }
    switch (letter) {
      case '0': speaker_setVolume(0); break;      
      case '1': speaker_setVolume(1); break;      
      case '2': speaker_setVolume(2); break;      
      case '3': speaker_setVolume(3); break;            
    }    
  }
}
