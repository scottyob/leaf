#ifndef PageMenuGPS_h
#define PageMenuGPS_h

#include <Arduino.h>
#include "menu_page.h"

class GPSMenuPage : public SettingsMenuPage {
  public:
    GPSMenuPage() {
      cursor_position = 0;
      cursor_max = 7;
    }
    void draw();

  protected:
    void setting_change(int8_t dir, uint8_t state, uint8_t count);

  private:
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