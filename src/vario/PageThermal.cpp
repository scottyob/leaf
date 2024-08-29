#include <Arduino.h>
#include <U8g2lib.h>

#include "display.h"
#include "fonts.h"
#include "displayFields.h"
#include "buttons.h"
#include "speaker.h"

#include "baro.h"
#include "power.h"
#include "tempRH.h"
#include "SDcard.h"
#include "log.h"


enum thermal_page_items { 
  cursor_thermalPage_none,
	cursor_thermalPage_alt1,
	//cursor_thermalPage_alt2,
	//cursor_thermalPage_userField1,
	//cursor_thermalPage_userField2,
	cursor_thermalPage_timer
};
int8_t cursor_position = cursor_thermalPage_none;
uint8_t cursor_max = 2;

void thermalPage_draw() {
  //baro_updateFakeNumbers();
  //gps_updateFakeNumbers();
  //display_update_temp_vars();

  u8g2.firstPage();
  do { 

		// Header Info ****************************************************
			// clock time
			u8g2.setFont(leaf_6x10);
			display_clockTime(0, 10, false);

			//heading
			u8g2.setFont(leaf_7x10);
			display_heading(40, 10, true);

			//battery 
			display_battIcon(89, 13, true);

			//speed
			u8g2.setFont(leaf_6x12);
			uint8_t x = display_speed(0,24);    // grab resulting x cursor value (if speed has 2 or 3 digits, things will shift over)
			u8g2.setFont(leaf_5h);
			u8g2.setCursor(x+2, 24);
			u8g2.print("MPH");
			
			
		// Main Info ****************************************************
			uint8_t topOfFrame = 30;
			uint8_t varioBarWidth = 20;
			uint8_t graphBoxHeight = 40;

			// Graph Box
			u8g2.drawFrame(varioBarWidth-1, topOfFrame, 96-varioBarWidth+1, graphBoxHeight);

			// Vario Bar
			display_varioBar(topOfFrame, 141, varioBarWidth, baro_getClimbRate());

			//air data
			display_alt_type(24, 90, leaf_8x14, alt_MSL);
			display_climbRatePointerBox(20, 92, 76, 17, 6, baro_getClimbRate());     // x, y, w, h, triangle size
			display_altAboveLaunch(24, 129, baro_getAltAboveLaunch());
		
			
		// User Fields ****************************************************
			uint8_t userFieldsTop = 137;
			uint8_t userFieldsHeight = 16;
			uint8_t userFieldsMid = userFieldsTop + userFieldsHeight;
			uint8_t userFieldsBottom = userFieldsMid + userFieldsHeight;
			uint8_t userSecondColumn = varioBarWidth/2+48;

			u8g2.drawHLine(varioBarWidth-1, userFieldsTop, 96-varioBarWidth+1);
			u8g2.drawHLine(varioBarWidth-1, userFieldsMid, 96-varioBarWidth+1);
			u8g2.drawHLine(varioBarWidth-1, userFieldsBottom, 96-varioBarWidth+1);
			u8g2.drawVLine(userSecondColumn, userFieldsTop, userFieldsHeight*2);



			display_temp(varioBarWidth+5, userFieldsMid-1, (int16_t)tempRH_getTemp());
			display_humidity(userSecondColumn+3, userFieldsMid-1, (uint8_t)tempRH_getHumidity());


    // Footer Info ****************************************************
		
			//flight timer (display selection box if selected)
			display_flightTimer(52, 191, 0, (cursor_position == cursor_thermalPage_timer));			

			// Bottom Status Icons	
				// SD Card Present
				char SDicon = 60;
				if(!SDcard_present()) SDicon = 61;
				u8g2.setCursor(12, 192);
				u8g2.setFont(leaf_icons);
				u8g2.print((char)SDicon);
			



		// Testing
		//u8g2.drawBox(8,174, 16, 16);



    
  } while ( u8g2.nextPage() ); 
  
}

void cursor_move(uint8_t button) {
	if (button == UP) {
		cursor_position--;
		if (cursor_position < 0) cursor_position = cursor_max;
	}
	if (button == DOWN) {
		cursor_position++;
  	if (cursor_position > cursor_max) cursor_position = 0;
	}
}


void thermalPage_button(uint8_t button, uint8_t state, uint8_t count) {

	

	switch (cursor_position) {
		case cursor_thermalPage_none:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) cursor_move(button);     					
					break;
				case RIGHT:
					if (state == RELEASED) {
						display_turnPage(page_next);
						speaker_playSound(fx_increase);
					}
					break;
				case LEFT:
					if (state == RELEASED) {
						display_turnPage(page_prev);
						speaker_playSound(fx_decrease);
					}
					break;
				case CENTER:
					break;
			}
			break;
		case cursor_thermalPage_alt1:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) cursor_move(button);     					
					break;
				case LEFT:
					break;
				case RIGHT:
					break;
				case CENTER:
					break;
			}
			break;
		/* case cursor_thermalPage_alt2:
			switch(button) {
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
			}
			break;
		case cursor_thermalPage_userField1:
			switch(button) {
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
			}
			break;
		case cursor_thermalPage_userField2:
			switch(button) {
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
			}
			break;
			*/
		case cursor_thermalPage_timer:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) cursor_move(button);     					
					break;
				case LEFT:
					break;
				case RIGHT:
					break;
				case CENTER:
						if (state == RELEASED) {
							flightTimer_toggle();
							cursor_position = cursor_thermalPage_none;
						}	else if (state == HELD) {
							flightTimer_reset();
							cursor_position = cursor_thermalPage_none;
						}
						
					break;
			}
			break;
	}
}