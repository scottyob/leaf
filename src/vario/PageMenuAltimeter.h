#ifndef PageMenuAltimeter_h
#define PageMenuAltimeter_h

#include <Arduino.h>
#include "menu_page.h"

class AltimeterMenuPage : public SettingsMenuPage {
  public:
    AltimeterMenuPage() {
      cursor_position = 0;
      cursor_max = 3;
    }
    void draw();

  protected:
    void setting_change(int8_t dir, uint8_t state, uint8_t count);

  private:
    static constexpr char * labels[4] = {
      "Back",
			"GPS Sync"
      "in Hg",
      "Alt Adj"      
    };
};


#endif
