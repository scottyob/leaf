#ifndef page_menu_units_h
#define page_menu_units_h

#include <Arduino.h>
#include "menu_page.h"

class VarioMenuPage : public SettingsMenuPage {
  public:
    VarioMenuPage() {
      cursor_position = 0;
      cursor_max = 8;
    }
    void draw();

  protected:
    void setting_change(int8_t dir, uint8_t state, uint8_t count);

  private:
    static constexpr char * labels[9] = {
      "Back",
      "BeepVol",
      "Sens",
      "Tones",
      "Lifty",
      "CliAv",
      "CliSt",
      "SnkAlr",
      "AltAdj"      
    };
};


#endif