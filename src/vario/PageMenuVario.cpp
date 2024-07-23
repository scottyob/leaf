#include <Arduino.h>
#include "PageMenuVario.h"
#include "pages.h"

#include "buttons.h"
#include "display.h"
#include "fonts.h"
#include "settings.h"
#include "baro.h"


enum vario_menu_items { 
  cursor_vario_back,
  cursor_vario_volume,
  cursor_vario_sensitive,
  cursor_vario_tones,
  cursor_vario_liftyair,
  cursor_vario_climbavg,
  cursor_vario_climbstart,
  cursor_vario_sinkalarm,
  cursor_vario_altadj

};


void VarioMenuPage::draw() {
  u8g2.firstPage();
  do { 
    // Title(s) 
    u8g2.setFont(leaf_6x12);
    u8g2.setCursor(2, 12);
    u8g2.setDrawColor(1);
    u8g2.print("UNITS");
    u8g2.drawHLine(0, 15, 64);

  // Menu Items
    uint8_t start_y = 29;
    uint8_t y_spacing = 16;
    uint8_t setting_name_x = 3;
    uint8_t setting_choice_x = 44;    
    uint8_t menu_items_y[] = {190, 45, 60, 75, 90, 105, 120, 135, 150};

    //first draw cursor selection box
    u8g2.drawRBox(setting_choice_x-2, menu_items_y[cursor_position]-14, 22, 16, 2);
    
      

    int32_t ALT_OFFSET;

    // then draw all the menu items
    for (int i = 0; i <= cursor_max; i++) {      
      u8g2.setCursor(setting_name_x, menu_items_y[i]);
      u8g2.print(labels[i]);
      u8g2.setCursor(setting_choice_x, menu_items_y[i]);
      if (i == cursor_position) u8g2.setDrawColor(0);
      else u8g2.setDrawColor(1);
      switch (i) {
        case cursor_vario_volume:
          u8g2.print(VOLUME_VARIO);
          break;
        case cursor_vario_sensitive:          
          u8g2.print(VARIO_AVERAGE);    
          break;
        case cursor_vario_tones:          
          u8g2.print("!!");
          break;
        case cursor_vario_liftyair:          
          u8g2.print(LIFTY_AIR);
          break;
        case cursor_vario_climbavg:          
          u8g2.print(CLIMB_AVERAGE);
          break;
        case cursor_vario_climbstart:          
          u8g2.print(CLIMB_START);
          break;
        case cursor_vario_sinkalarm:          
          u8g2.print(SINK_ALARM);
          break;
        case cursor_vario_altadj:
          u8g2.print(baro_getAlt()+ALT_OFFSET, 0);
          break;
        case cursor_vario_back:
          u8g2.print((char)124);
          break;        
      }
    u8g2.setDrawColor(1);
    }
  } while ( u8g2.nextPage() ); 
}


void VarioMenuPage::setting_change(int8_t dir) {
  switch (cursor_position) {
    case cursor_vario_volume:
      settings_toggleUnits(&UNITS_alt);
      break;
    case cursor_vario_sensitive:
      settings_toggleUnits(&UNITS_climb);
      break;
    case cursor_vario_tones:
      settings_toggleUnits(&UNITS_speed);
      break;
    case cursor_vario_liftyair:
      settings_toggleUnits(&UNITS_distance);
      break;
    case cursor_vario_climbavg:
      settings_toggleUnits(&UNITS_heading);
      break;
    case cursor_vario_climbstart:
      settings_toggleUnits(&UNITS_temp);
      break;
    case cursor_vario_sinkalarm:
      settings_toggleUnits(&UNITS_hours);
      break;
    case cursor_vario_altadj:
      break;
    case cursor_vario_back:
      //if (dir == 0) 
      display_turnPage(page_back);
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