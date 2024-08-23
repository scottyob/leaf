#ifndef buttons_h
#define buttons_h

#include <Arduino.h>


//Pinout for Leaf V3.2.0
#define BUTTON_PIN_CENTER  2  // INPUT
#define BUTTON_PIN_LEFT    3  // INPUT
#define BUTTON_PIN_DOWN    4  // INPUT
#define BUTTON_PIN_UP      5  // INPUT
#define BUTTON_PIN_RIGHT   6 // INPUT

/*
//Pinout for Breadboard
#define BUTTON_PIN_UP     40  
#define BUTTON_PIN_DOWN   38
#define BUTTON_PIN_LEFT   39
#define BUTTON_PIN_RIGHT  42
#define BUTTON_PIN_CENTER 41
*/

enum buttons {NONE, UP, DOWN, LEFT, RIGHT, CENTER, BOUNCE};
enum button_states {NO_STATE, PRESSED, RELEASED, HELD, HELD_LONG};

uint8_t buttons_init(void);

uint8_t buttons_check(void);
uint8_t buttons_inspectPins(void);
uint8_t buttons_debounce(uint8_t button);
uint8_t buttons_get_state(void);
uint16_t buttons_get_hold_count(void);

uint8_t buttons_update(void); // the main task of checking and handling button pushes


#endif 