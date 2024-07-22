#ifndef PageMenuGPS_h
#define PageMenuGPS_h

#include <Arduino.h>
#include "menu_page.h"

class GPSMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif