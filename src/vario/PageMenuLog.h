#ifndef PageMenuLog_h
#define PageMenuLog_h

#include <Arduino.h>
#include "menu_page.h"

class LogMenuPage : public MenuPage {
  public:
    LogMenuPage() {
      cursor_position = 0;
      cursor_max = 7;
    }
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
  private:
    void setting_change(int8_t dir);
    static constexpr char * labels[8] = {
      "Back",
      "Alt:",
      "Climb:",
      "Speed:",
      "Dist:",
      "Head:",
      "Temp:",
      "Time:"
    };
};


#endif