#ifndef PageMenuSystem_h
#define PageMenuSystem_h

#include <Arduino.h>
#include "menu_page.h"

class SystemMenuPage : public SettingsMenuPage {
  public:
    SystemMenuPage() {
      cursor_position = 0;
      cursor_max = 7;
    }
    void draw();

  protected:
    void setting_change(int8_t dir);

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