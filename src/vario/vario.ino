#include <Arduino.h>
#include <HardwareSerial.h>

#include "baro.h"
#include "Leaf_SPI.h"
#include "speaker.h"
#include "display.h"
#include "gps.h"
#include "buttons.h"
#include "IMU.h"

uint8_t display_page = 0;

//should move to SPI file later
//#define LCD_RS    46          // RS pin for data or instruction
//#define GLCD_RS    9          // RS pin for data or instruction
//#define GLCD_RESET 7          // Reset pin
//#define LED_PIN    6



// Main Loop & Task Manager
/*
The main loop perioritizes processesing any data in the serial buffer so as not to miss any NMEA GPS characters.  
The main loop will continue looping & processing serial data until all NMEA sentences have been captured.
Then, if there is no more serial data, any other remaining tasks will be processed.
Once all tasks are done, and if the serial UART is quiet, then the processor will go to sleep.

Every 10ms, driven by an interrupt timer, the system will wake up, set flags for which tasks are needed, and then run the main loop (i.e., pick up in the loop where it left off when it went to sleep)

TODO: In addition to the timer-driven interrupt, we may consider also setting wake-up interrupts for the pushbuttons, the GPS 1PPS signal, and perhaps others.
*/

// Trackers for Task Manager Queue.  Default to tasks being needed, so they execute upon startup
char taskman_buttons = 1;   // poll & process buttons
char taskman_baro = 1;      // (1) Process on-chip ADC pressure, (2) read pressure and process on-chip ADC temperature, (3) calculate, filter, and store values
char taskman_imu = 1;       // read sensors and update values
char taskman_gps = 1;       // process any NMEA strings and update values
char taskman_lcd = 1;       // update display
char taskman_power = 1;     // check battery, check auto-turn-off, etc
char taskman_logging = 1;   // check auto-start, increment timers, update log file, etc
char taskman_setTasks = 1;  // the task of setting tasks -- usually set by timer-driven interrupt

// Counters for system task timer
char counter_10ms_block = 0;
char counter_100ms_block = 0;




/////////////////////////////////////////////////
// Main Loop for Processing Tasks ///////////////
/////////////////////////////////////////////////
void main_timer_loop() {    
  if (taskman_setTasks) {   // if we're running through this loop for the first time in 10ms (since the last timer driven interrupt)
    setTasks();             // then set necessary tasks
    taskman_setTasks = 0;   // we don't need to do this again until the next 10ms timer interrupt
  }

  //if (serial is available) uart_is_quiet = process_serial_stuff(); // serial is high priority so we don't overflow the buffer, first check this if available
  //else { taskManager(); }                                          // otherwise do other remaining tasks
  //if (uart_is_quiet) goToSleep();                 
  // else, run this loop again and keep processing the serial buffer until we're done with all the NMEA sentences this cycle
}


void setTasks(void) {  
  // increment time counters
  if(++counter_10ms_block = 10) {
    counter_10ms_block = 0;           // every 10 periods of 10ms, go back to 0 (100ms total)
    if (++counter_100ms_block = 10) {
      counter_100ms_block = 0;        // every 10 periods of 100ms, go back to 0 (1sec total)
    }
  }

  // TODO: check buttons here (every 10ms)

  // set additional tasks to complete, broken down into 10ms block cycles.  (embedded if() statements allow for tasks every second)
  switch (counter_10ms_block) {
    case 0:
      taskman_baro = 1;  // update baro every 50ms on the 0th and 5th blocks

      // cycle through tasks needed less often (every 500ms - 1000ms)
      if (counter_100ms_block == 0) taskman_gps = 1;                              // every second: gps
      if (counter_100ms_block == 1) taskman_power = 1;                            // every second: power checks      
      if (counter_100ms_block == 2) taskman_logging = 1;                          // every second: logging
      if (counter_100ms_block == 3 || counter_100ms_block == 8) taskman_lcd = 1;  // every half-second LCD
      break;
    case 1:
      // FYI: baro step 2 will update here
      break;
    case 2:
      // FYI: baro step 3 will update here
      break;
    case 3:
      // FYI: baro step 4 will update here
      break;
    case 4:
      taskman_imu = 1;  // update accel every 100ms during the 4th block
      break;
    case 5:
      taskman_baro = 1;  // update baro every 50ms on the 0th and 5th blocks
      break;
    case 6:
      // FYI: baro step 2 will update here
      break;
    case 7:
      // FYI: baro step 3 will update here
      break;
    case 8:
      // FYI: baro step 4 will update here
      break;
    case 9:
      break;      
  }
}

char process_serial_stuff() {
  char finished_sending = 0;
  //grab_stuff_from_serial_buffer();
  //process_serial_GPS_NMEA_strings();
  //if (GPS_is_done_with_its_strings) finished_sending = 1;
  return finished_sending;
}

// execute necessary tasks while we're awake and have things to do
void taskManager(void) {
  /*
  if (taskman_buttons) buttons_update();
  if (taskman_baro) taskman_baro = baro_update(taskman_baro);    // do bare update if needed, and prep for next step in the update process
  if (taskman_imu) imu_update();
  if (taskman_gps) gps_update();
  if (taskman_lcd) lcd_update();
  if (taskman_power) power_update();
  if (taskman_logging) logging_update();
  */
}




// Main Timer Setup and interrupt event
hw_timer_t *Timer0_Cfg = NULL;


void IRAM_ATTR Timer0_ISR() {
  //do stuff every alarm cycle (default 10ms)
  taskman_setTasks = 1; // next time through main loop, set tasks to do!
  // wakeup();  // go back to main loop and keep processing stuff that needs doing!
}


void setup()
{

// Start USB Serial Debugging Port
Serial.begin(115200);
Serial.println("Starting Setup");

/*
//Start Main System Timer for Interrupt Events
  Timer0_Cfg = timerBegin(0, 80, true);                 // Prescaler of 80, so 80Mhz drops to 1Mhz
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);  // Attach interrupt to handle alarm events
  timerAlarmWrite(Timer0_Cfg, 100000, true);            // Set alarm to go every 100,000 ticks (every 0.1 seconds)
  timerAlarmEnable(Timer0_Cfg);                         // Enable alarm & timer
*/


//Initialize devices
setup_Leaf_SPI();
Serial.println("Finished SPI");
GLCD_init();
Serial.println("Finished GLCD");
buttons_init();
Serial.println("Finished buttons");
baro_init();
Serial.println("Finished Baro");
imu_init();
Serial.println("Finished IMU");
display_init();
Serial.println("Finished display");
gps_init();
Serial.println("Finished GPS");
speaker_init();
Serial.println("Finished Speaker");
Serial.println("Finished Setup");


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  
  uint8_t button = buttons_check();
  uint8_t button_state = buttons_get_state();

  if (button_state == PRESSED) {
    if (button == LEFT) {      
      if (display_page > 0) display_page--;
    } else if (button == RIGHT) {
      display_page++;
      if (display_page > 5) display_page = 5;
    }
    Serial.print("Going to page: ");
    Serial.println(display_page);
  }

  switch (display_page) {
    case 0:
      gps_test_sats();
      break;
    case 1:
      display_test();
      break;
    case 2:
      speaker_TEST();
      break;
    case 3:
      imu_test();
      break;
    case 4:
      display_test_big(1);
      break;
    case 5:
      display_test_big(2);
      break;
  }
  //
  //

  //


/*
pressure_update1();
delay(15);
pressure_update2();
delay(15);
pressure_update3();
pressure_update4();


Serial.print("T: ");
Serial.print(TEMP);
Serial.print("  T_filt: ");
Serial.print(TEMPfiltered);
Serial.print("  P: ");
Serial.print(PRESSURE);
Serial.print("  P_filt: ");
Serial.print(PRESSUREfiltered);
Serial.print("  Alt: ");
Serial.print(P_ALT);
Serial.print("  Alt_filt: ");
Serial.print(P_ALTfiltered);
Serial.println(" ");
delay(450);
*/

}


