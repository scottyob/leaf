#ifndef PageThermalSimple_h
#define PageThermalSimple_h

#include <Arduino.h>



// draw the pixels to the display
void thermalSimplePage_draw(void);

// handle button presses relative to what's shown on the display
void thermalSimplePage_button(uint8_t button, uint8_t state, uint8_t count);


#endif