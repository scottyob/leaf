#ifndef PageMenuSystem_h
#define PageMenuSystem_h

#include <Arduino.h>
#include "menu_page.h"

class SystemMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif