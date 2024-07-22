#ifndef PageMenuLog_h
#define PageMenuLog_h

#include <Arduino.h>
#include "menu_page.h"

class LogMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif