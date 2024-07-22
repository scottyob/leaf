#ifndef PageMenuUnits_h
#define PageMenuUnits_h

#include <Arduino.h>
#include "menu_page.h"

class UnitsMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif