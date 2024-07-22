#ifndef PageMenuDisplay_h
#define PageMenuDisplay_h

#include <Arduino.h>
#include "menu_page.h"

class DisplayMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif