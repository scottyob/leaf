#ifndef buttons_h
#define buttons_h

#include <Arduino.h>

#define BUTTON_PIN_UP     40  
#define BUTTON_PIN_DOWN   38
#define BUTTON_PIN_LEFT   39
#define BUTTON_PIN_RIGHT  42
#define BUTTON_PIN_CENTER 41


enum buttons {NONE, UP, DOWN, LEFT, RIGHT, CENTER, BOUNCE};
enum button_state {NO_STATE, PRESSED, RELEASED, HELD, HELD_LONG};

void buttons_init(void);
uint8_t buttons_check(void);
uint8_t buttons_get_state(void);
uint8_t buttons_debounce(void);


#endif 