#include <Arduino.h>
#include "PageMenuAltimeter.h"
#include "pages.h"

#include "buttons.h"
#include "display.h"
#include "fonts.h"
#include "settings.h"
#include "baro.h"
#include "gps.h"
#include "speaker.h"


enum altimeter_menu_items { 
  cursor_altimeter_back,
	cursor_altimeter_trackGPSAlt,
  cursor_altimeter_baroSetting,
	cursor_altimeter_adjust
};


void AltimeterMenuPage::draw() {
  u8g2.firstPage();
  do { 
    // Title(s) 
    u8g2.setFont(leaf_6x12);
    u8g2.setCursor(2, 12);
    u8g2.setDrawColor(1);
    u8g2.print("ALTIMETER");
    u8g2.drawHLine(0, 15, 64);

  // Menu Items
    uint8_t start_y = 29;
    uint8_t y_spacing = 16;
    uint8_t setting_name_x = 2;
    uint8_t setting_choice_x = 74;    
    uint8_t menu_items_y[] = {190, 75, 90, 105};

    //first draw cursor selection box
    u8g2.drawRBox(setting_choice_x-2, menu_items_y[cursor_position]-14, 24, 16, 2);


    // then draw all the menu items
    for (int i = 0; i <= cursor_max; i++) {      
      u8g2.setCursor(setting_name_x, menu_items_y[i]);
      u8g2.print(labels[i]);
      u8g2.setCursor(setting_choice_x, menu_items_y[i]);
      if (i == cursor_position) u8g2.setDrawColor(0);
      else u8g2.setDrawColor(1);
      switch (i) {
				case cursor_altimeter_trackGPSAlt:          
          if (ALT_SYNC_GPS) u8g2.print("YES");
					else u8g2.print("NO");
          break;
        case cursor_altimeter_baroSetting:       
          u8g2.print(baroAltimeterSetting);
          break;
        case cursor_altimeter_adjust:
          u8g2.print("-/+");
          break;
        case cursor_altimeter_back:
          u8g2.print((char)124);
          break;        
      }
    u8g2.setDrawColor(1);    
    }
    // show altitude so proper offset can be set, if desired
    display_alt(5, menu_items_y[cursor_max]+15, leaf_6x12, baro_getOffsetAlt());
  } while ( u8g2.nextPage() ); 
}


void AltimeterMenuPage::setting_change(int8_t dir, uint8_t state, uint8_t count) {  
  switch (cursor_position) {
		case cursor_altimeter_trackGPSAlt:
      if (state == RELEASED) settings_toggleBoolNeutral(&ALT_SYNC_GPS);
      break;
    case cursor_altimeter_baroSetting:
      if (state == RELEASED) settings_adjustVolumeVario(dir);
      break;
    case cursor_altimeter_adjust:
      if (dir == 0 && count == 1 && state == HELD) {  // if center button held for 1 'action time'
        if (settings_matchGPSAlt()) { // successful reset of AltOffset to match GPS altitude
          speaker_playSound(fx_enter);  
        } else {                      // unsuccessful 
          speaker_playSound(fx_cancel);
        }
      } else if (dir != 0) {
        if (state == PRESSED || state == HELD || state == HELD_LONG) {
          baro_adjustAltOffset(dir, count);
					//settings_adjustAltOffset(dir, count);
          speaker_playSound(fx_neutral);
        }
      }
      break;
    case cursor_altimeter_back:     
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