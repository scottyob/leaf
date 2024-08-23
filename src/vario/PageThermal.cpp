#include <Arduino.h>
#include <U8g2lib.h>

#include "display.h"
#include "fonts.h"
#include "displayFields.h"

#include "baro.h"
#include "power.h"



enum thermal_page_items { 
  cursor_thermalPage_none,
  cursor_thermalPage_timer,
	cursor_thermalPage_userField1,
	cursor_thermalPage_userField2,


  cursor_vario_sensitive,
  cursor_vario_tones,
  cursor_vario_liftyair,
  cursor_vario_climbavg,
  cursor_vario_climbstart,
  cursor_vario_sinkalarm,
  cursor_vario_altadj

};



void thermalPage_draw() {
  //baro_updateFakeNumbers();
  //gps_updateFakeNumbers();
  //display_update_temp_vars();

  u8g2.firstPage();
  do { 

		// clock time
		display_clockTime(0, 12, false);

		//heading
    display_headingTurn(40, 10);

		//battery 
		display_battIcon(89, 13, true);

		//speed
    u8g2.setFont(leaf_6x12);
    uint8_t x = display_speed(0,24);    // grab resulting x cursor value (if speed has 2 or 3 digits, things will shift over)
		u8g2.setFont(leaf_5h);
		u8g2.setCursor(x+2, 24);
		u8g2.print("MPH");
		
		// Vario
		display_varioBar(30, 149, 20, baro_getClimbRate());
    display_climbRatePointerBox(20, 100, 76, 17, 6, baro_getClimbRate());     // x, y, w, h, triangle size
    
    //air data
    display_alt(24, 93, leaf_8x14, baro_getAlt());
    display_altAboveLaunch(24, 135, baro_getAltAboveLaunch());
    
		
		
    display_temp(23, 156, baro_getTemp());

    //flight timer
    display_flightTimer(2, 192, 0);
    display_flightTimer(48, 192, 1);
    
  } while ( u8g2.nextPage() ); 
  
}


void thermalPage_button(uint8_t button, uint8_t state, uint8_t count) {

}