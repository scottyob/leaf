#ifndef PageMenuMain_h
#define PageMenuMain_h

#include <Arduino.h>
#include "menu_page.h"


void draw_main_menu(void);

class MainMenuPage : public MenuPage {
  public:
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
};


#endif