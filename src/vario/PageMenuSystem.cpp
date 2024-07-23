#include <Arduino.h>
#include "PageMenuSystem.h"
#include "pages.h"

#include "buttons.h"
#include "display.h"
#include "fonts.h"
#include "settings.h"
#include "power.h"
#include "speaker.h"


enum system_menu_items { 
  cursor_system_back,
  cursor_system_timezone,
  cursor_system_volume,
  cursor_system_poweroff,
  cursor_system_charge,
  cursor_system_wifi,
  cursor_system_bluetooth,
  cursor_system_reset
};


void SystemMenuPage::draw() {

  int16_t displayTimeZone = TIME_ZONE;

  u8g2.firstPage();
  do { 
    // Title(s) 
    u8g2.setFont(leaf_6x12);
    u8g2.setCursor(2, 12);
    u8g2.setDrawColor(1);
    u8g2.print("SYSTEM");
    u8g2.drawHLine(0, 15, 64);

  // Menu Items
    uint8_t start_y = 29;
    uint8_t y_spacing = 16;
    uint8_t setting_name_x = 3;
    uint8_t setting_choice_x = 38;    
    uint8_t menu_items_y[] = {190, 45, 60, 75, 90, 105, 120, 135};

    //first draw cursor selection box
    u8g2.drawRBox(setting_choice_x-2, menu_items_y[cursor_position]-14, 22, 16, 2);
    
    // then draw all the menu items
    for (int i = 0; i <= cursor_max; i++) {      
      u8g2.setCursor(setting_name_x, menu_items_y[i]);
      u8g2.print(labels[i]);
      u8g2.setCursor(setting_choice_x, menu_items_y[i]);
      if (i == cursor_position) u8g2.setDrawColor(0);
      else u8g2.setDrawColor(1);
      switch (i) {
        case cursor_system_timezone:      
          // sign
          if (displayTimeZone < 0) {
            u8g2.print('-');
            displayTimeZone *= -1;
          } else {
            u8g2.print('+');
          }
          //hours, :, minute
          u8g2.print(displayTimeZone/60);
          u8g2.print(':');
          if (displayTimeZone % 60 == 0) u8g2.print("00");
          else u8g2.print(displayTimeZone % 60);
          break;
        case cursor_system_volume:
          u8g2.print(VOLUME_SYSTEM); 
          break;
        case cursor_system_poweroff:
          if (AUTO_OFF) u8g2.print("ON");
          else u8g2.print("OFF");
          break;
        case cursor_system_charge:
          if (power_getInputCurrent()) u8g2.print("LOW");
          else if (power_getInputCurrent()) u8g2.print("MED");
          else if (power_getInputCurrent()) u8g2.print("HI");
          else if (power_getInputCurrent()) u8g2.print("OFF");
          break;
        case cursor_system_wifi:
          if (WIFI_ON) u8g2.print("ON");
          else u8g2.print("OFF");
          break;
        case cursor_system_bluetooth:
          if (BLUETOOTH_ON) u8g2.print("ON");
          else u8g2.print("OFF");
          break;
        case cursor_system_reset:
          u8g2.print((char)126);
          break;
        case cursor_system_back:
          u8g2.print((char)124);
          break;        
      }
    u8g2.setDrawColor(1);
    }
  } while ( u8g2.nextPage() ); 
}


void SystemMenuPage::setting_change(int8_t dir, uint8_t state, uint8_t count) {
  switch (cursor_position) {
    case cursor_system_timezone:
      if (state == RELEASED && dir != 0) settings_adjustTimeZone(dir);
      if (state == HELD && dir == 0) settings_adjustTimeZone(dir);
      break;
    case cursor_system_volume:
      if (state == RELEASED && dir != 0) settings_adjustVolumeSystem(dir);
      break;
    case cursor_system_poweroff:
      if (state == RELEASED) settings_toggleBoolOnOff(&AUTO_OFF);
      break;
    case cursor_system_charge:
      if (state == RELEASED) 
      break;
    case cursor_system_wifi:
      if (state == RELEASED) 
      break;
    case cursor_system_bluetooth:
      if (state == RELEASED) 
      break;
    case cursor_system_reset:
      if (state == RELEASED) 
      break;
    case cursor_system_back:      
      if (state == RELEASED) mainMenuPage.backToMainMenu();
      break;
  }
}


// helpful switch constructors to copy-paste as needed:
/*
switch (button) {
  case UP:
    break;
  case DOWN:
    break;
  case LEFT:
    break;
  case RIGHT:
    break;
  case CENTER:
    break;
*/

/*
switch (state) {
  case RELEASED:
    break;
  case PRESSED:
    break;
  case HELD:
    break;
  case HELD_LONG:
    break;
}
*/