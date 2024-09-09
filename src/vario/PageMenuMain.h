#ifndef PageMenuMain_h
#define PageMenuMain_h

#include <Arduino.h>
#include "menu_page.h"


class MainMenuPage : public MenuPage {
  public:
    MainMenuPage() {
      cursor_position = 0;
      cursor_max = 7;
    }
    bool button_event(uint8_t button, uint8_t state, uint8_t count);
    void draw();
    void backToMainMenu();
    void quitMenu();

  private:
    void draw_main_menu();
    bool mainMenuButtonEvent(uint8_t button, uint8_t state, uint8_t count);
    void menu_item_action(int8_t dir);
    static constexpr char * labels[8] = {
      "Back",
      "Altimeter",
      "Vario",
      "Display",
      "Units",
      "GPS",
      "Log",
      "System"  
    };
};


#endif