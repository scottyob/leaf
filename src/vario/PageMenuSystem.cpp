#include <Arduino.h>
#include "PageMenuSystem.h"
#include "pages.h"

#include "buttons.h"
#include "display.h"
#include "fonts.h"
#include "settings.h"
#include "power.h"
#include "speaker.h"
#include "PageMenuSystemWifi.h"


enum system_menu_items { 
  cursor_system_back,
  cursor_system_timezone,
  cursor_system_volume,
  cursor_system_poweroff,
  cursor_system_charge,
  cursor_system_ecomode,
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
    u8g2.drawHLine(0, 15, 95);

  // Menu Items
    uint8_t start_y = 29;
    uint8_t y_spacing = 16;
    uint8_t setting_name_x = 2;
    uint8_t setting_choice_x = 64;    
    uint8_t menu_items_y[] = {190, 45, 60, 75, 90, 105, 120, 135, 150};
    char twoZeros[] = "00";

    //first draw cursor selection box
    u8g2.drawRBox(setting_choice_x-2, menu_items_y[cursor_position]-14, 34, 16, 2);
    
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
          if (displayTimeZone % 60 == 0) u8g2.print(twoZeros);
          else u8g2.print(displayTimeZone % 60);
          break;

        case cursor_system_volume:
          u8g2.setCursor(setting_choice_x + 12, menu_items_y[i]);
          u8g2.print(VOLUME_SYSTEM); 
          break;

        case cursor_system_poweroff:
          u8g2.setCursor(setting_choice_x + 4, menu_items_y[i]);
          if (AUTO_OFF) u8g2.print("ON");
          else u8g2.print("OFF");
          break;

        case cursor_system_charge:
          u8g2.setCursor(setting_choice_x + 5, menu_items_y[i]);
          if (power_getInputCurrent() == i100mA) u8g2.print("100");
          else if (power_getInputCurrent() == i500mA) u8g2.print("500");
          else if (power_getInputCurrent() == iMax) u8g2.print("MAX");
          else if (power_getInputCurrent() == iStandby) u8g2.print("OFF");
          break;

        case cursor_system_ecomode:
          u8g2.setCursor(setting_choice_x + 4, menu_items_y[i]);
          if (ECO_MODE) u8g2.print("ON");
          else u8g2.print("OFF");
          break;

        case cursor_system_wifi:
          u8g2.setCursor(setting_choice_x + 4, menu_items_y[i]);
          if (WIFI_ON) u8g2.print("ON");
          else u8g2.print("OFF");
          break;

        case cursor_system_bluetooth:
          u8g2.setCursor(setting_choice_x + 4, menu_items_y[i]);
          if (BLUETOOTH_ON) u8g2.print("ON");
          else u8g2.print("OFF");
          break;

        case cursor_system_reset:
          u8g2.setCursor(setting_choice_x + 8, menu_items_y[i]);
          u8g2.print((char)126);
          break;

        case cursor_system_back:
          u8g2.setCursor(setting_choice_x + 8, menu_items_y[i]);
          u8g2.print((char)124);
          break;        
      }
    u8g2.setDrawColor(1);
    }
  } while ( u8g2.nextPage() ); 
}


void SystemMenuPage::setting_change(Button dir, ButtonState state, uint8_t count) {
  bool redraw = false;
  switch (cursor_position) {
    case cursor_system_timezone:
      if (state == RELEASED && dir != Button::NONE) settings_adjustTimeZone(dir);
      if (state == HELD && dir == Button::NONE) settings_adjustTimeZone(dir);
      break;
    case cursor_system_volume:
      if (state == RELEASED && dir != Button::NONE) settings_adjustVolumeSystem(dir);
      break;
    case cursor_system_poweroff:
      if (state == RELEASED) settings_toggleBoolOnOff(&AUTO_OFF);
      break;
    case cursor_system_charge:
      if (state == RELEASED) {}
      break;
    case cursor_system_ecomode:
      if (state == RELEASED) settings_toggleBoolOnOff(&ECO_MODE);
      break;
    case cursor_system_wifi:
      if (state != RELEASED) break;

      // User has selected WiFi, show this page
      static PageMenuSystemWifi wifiPage;
      push_page(&wifiPage);
      redraw = true;
      break;
    case cursor_system_bluetooth:
      if (state == RELEASED) {}
      break;
    case cursor_system_reset:
      if (state == RELEASED) {}
      if (state == HELD) {
        settings_reset();
      }
      break;
    case cursor_system_back:        
      if (state == RELEASED) {
        speaker_playSound(fx_cancel);
        settings_save(); 
        mainMenuPage.backToMainMenu();
      } else if (state == HELD) {
        speaker_playSound(fx_exit);
        settings_save(); 
        mainMenuPage.quitMenu();        
      }      
      break;
  }
}


// helpful switch constructors to copy-paste as needed:
/*
switch (button) {
  case Button::UP:
    break;
  case Button::DOWN:
    break;
  case Button::LEFT:
    break;
  case Button::RIGHT:
    break;
  case Button::CENTER:
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