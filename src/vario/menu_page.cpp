#include "menu_page.h"
#include "buttons.h"

void MenuPage::cursor_prev() {
  cursor_position--;
  if (cursor_position < 0) cursor_position = cursor_max;
}

void MenuPage::cursor_next() {
  cursor_position++;
  if (cursor_position > cursor_max) cursor_position = 0;
}

bool SettingsMenuPage::button_event(uint8_t button, uint8_t state, uint8_t count) {    
  switch (button) {
    case UP:
      if (state == RELEASED) cursor_prev();
      break;
    case DOWN:
      if (state == RELEASED) cursor_next();
      break;
    case LEFT:
      setting_change(-1, state, count);
      break;
    case RIGHT:
      setting_change(1, state, count);
      break;
    case CENTER:
      setting_change(0, state, count);
      break;    
  }    
  return true;   //update display after button push so that the UI reflects any changes immediately
}
