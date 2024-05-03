/*
 * buttons.cpp
 *
 * 5-way joystick switch (UP DOWN LEFT RIGHT CENTER)
 * Buttons are active-high, via resistor dividers fed from "SYSPOWER", which is typically 4.4V (supply from Battery Charger under USB power) or Battery voltage (when no USB power is applied)
 * Use internal pull-down resistors on button pins 
 *
 */
#include <Arduino.h>
#include "buttons.h"

//button debouncing 
uint8_t button_debounce_last = NONE;
uint32_t button_debounce_time = 5;    // time in ms for stabilized button state before returning the button press
uint32_t button_time_initial = 0;
uint32_t button_time_elapsed = 0;

//button actions
uint8_t button_last = NONE;
uint8_t button_state = NO_STATE;
uint16_t button_min_hold_time = 800;  // time in ms to count a button as "held down"
uint16_t button_max_hold_time = 3500; // time in ms to start further actions on long-holds

uint16_t button_hold_action_time_initial = 0;
uint16_t button_hold_action_time_elapsed = 0; // counting time between 'action steps' while holding the button
uint16_t button_hold_action_time_limit = 500; // time in ms required between "action steps" while holding the button
uint16_t button_hold_counter = 0;

void buttons_init(void) {

  //configure pins
  pinMode(BUTTON_PIN_UP, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN_DOWN, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN_LEFT, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN_RIGHT, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN_CENTER, INPUT_PULLDOWN);

}

uint8_t buttons_get_state(void) {
  return button_state;
}

uint8_t buttons_get_hold_count(void) {
  return button_hold_counter;
}


uint8_t buttons_check(void) {

  uint8_t button = buttons_debounce();  // first check if button press is in a stable state
  
  //Serial.print("debounced: ");
  //Serial.println(button);

  // reset and exit if bouncing
  if (button == BOUNCE) {
    button_state = NO_STATE;
    button_hold_counter = 0;
    return NONE;
  }

  // if we have a state change (low to high or high to low)
  if (button != button_last) { 

    button_hold_counter = 0;

    if (button != NONE) {
      button_state = PRESSED;       
    } else {      
      button_state = RELEASED;
      button = button_last;       // we are presently seeing "NONE", which is the release of the previously pressed/held button, so grab that previous button            
    }
  // otherwise we have a non-state change (button is held)
  } else if (button != NONE) {
    if (button_time_elapsed >= button_max_hold_time) {
      button_state = HELD_LONG;
    } else if (button_time_elapsed >= button_min_hold_time) {
      button_state = HELD;
    } else {
      button_state = NO_STATE;
    }   
  } else {
    button_state = NO_STATE;
  }

  // only "act" on a held button every ~500ms (button_hold_action_time_limit)
  if (button_state == HELD || button_state == HELD_LONG) {
    button_hold_action_time_elapsed = millis() - button_hold_action_time_initial;
    if (button_hold_action_time_elapsed < button_hold_action_time_limit) {
      button_state = NO_STATE;
    } else {
      button_hold_counter++;
      button_hold_action_time_elapsed = 0;
      button_hold_action_time_initial = millis();    
    }
  }

/*
Serial.print("button: ");
Serial.print(button);
Serial.print(" state: ");
Serial.print(button_state);
Serial.print(" count: ");
Serial.println(button_hold_counter);
*/



  if(button_state != NO_STATE) {
    switch(button) {
      case CENTER:
        Serial.print("button: CENTER");
        break;
      case LEFT:
        Serial.print("button: LEFT  ");
        break;
      case RIGHT:
        Serial.print("button: RIGHT ");
        break;
      case UP:
        Serial.print("button: UP    ");
        break;
      case DOWN:
        Serial.print("button: DOWN  ");
        break;
      case NONE: 
        Serial.print("button: NONE  ");     
        break;
    }

    switch(button_state) {
      case PRESSED:
        Serial.print(" state: PRESSED  ");
        break;
      case RELEASED:
        Serial.print(" state: RELEASED ");
        break;
      case HELD:
        Serial.print(" state: HELD     ");
        break;
      case HELD_LONG:
        Serial.print(" state: HELD_LONG");
    }

    Serial.print(" hold count: ");
    Serial.println(button_hold_counter);   
  } 


  //save button to track for next time.
  // ..if state is RELEASED, then button_last should be 'NONE', since we're seeing the falling edge of the previous button press
  if (button_state == RELEASED) {
    button_last = NONE;
  } else {
    button_last = button;
  }
  return button;
}

// check for a minimal stable time before asserting that a button has been pressed or released
uint8_t buttons_debounce(void) {

  // check the state of the hardware buttons
  int button = NONE;
  if (digitalRead(BUTTON_PIN_UP) == HIGH) button = UP;
  if (digitalRead(BUTTON_PIN_DOWN) == HIGH) button = DOWN;
  if (digitalRead(BUTTON_PIN_LEFT) == HIGH) button = LEFT;
  if (digitalRead(BUTTON_PIN_RIGHT) == HIGH) button = RIGHT;
  if (digitalRead(BUTTON_PIN_CENTER) == HIGH) button = CENTER;  
  
  if (button != button_debounce_last) {                   // if this is a new button state
    button_time_initial = millis();                       // capture the initial start time
    button_time_elapsed = 0;                              // and reset the elapsed time
    button_debounce_last = button;
  } else {                                                // this is the same button as last time, so calculate the duration of the press
    button_time_elapsed = millis() - button_time_initial; // (the roll-over modulus math works on this)
    if (button_time_elapsed >= button_debounce_time) {        
      return button;
    }
  }
  return BOUNCE;
}

