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
#include "IMU.h"
#include "gps.h"
#include "settings.h"
#include "gpx.h"


enum navigate_page_items { 
  cursor_navigatePage_none,
	cursor_navigatePage_alt1,
	cursor_navigatePage_waypoint,
	//cursor_navigatePage_userField1,
	//cursor_navigatePage_userField2,
	cursor_navigatePage_timer
};
uint8_t navigatePage_cursorMax = 3;

int8_t navigatePage_cursorPosition = cursor_navigatePage_none;
uint8_t navigatePage_cursorTimeCount = 0;	// count up for every page_draw, and if no button is pressed, then reset cursor to "none" after the timeOut value is reached.
uint8_t navigatePage_cursorTimeOut = 12;	// after 12 page draws (6 seconds) reset the cursor if a button hasn't been pushed.




void navigatePage_draw() {

	// if cursor is selecting something, count toward the timeOut value before we reset cursor
	if (navigatePage_cursorPosition != cursor_navigatePage_none && navigatePage_cursorTimeCount++ >= navigatePage_cursorTimeOut) {
		navigatePage_cursorPosition = cursor_navigatePage_none;
		navigatePage_cursorTimeCount = 0;		
	}

  u8g2.firstPage();
  do { 

		// Header Info ****************************************************
			// clock time
			u8g2.setFont(leaf_6x10);
			display_clockTime(0, 10, false);

			//heading
			u8g2.setFont(leaf_7x10);
			display_headingTurn(40, 10);

			//battery 
			display_battIcon(89, 13, true);

			//speed
			u8g2.setFont(leaf_6x12);
			uint8_t x = display_speed(0,24);    // grab resulting x cursor value (if speed has 2 or 3 digits, things will shift over)
			u8g2.setFont(leaf_5h);
			u8g2.setCursor(x+2, 24);
			u8g2.print("MPH");
		
		///////////////////////////////////////////////////
		// Nav Circle

			// Nav Circles Locations
			uint8_t nav_x = 57;
			uint8_t nav_y = 52;
			uint8_t nav_r = 37;
			uint8_t wind_r = 8;

			u8g2.drawCircle(nav_x, nav_y, nav_r);
			u8g2.drawCircle(nav_x, nav_y, nav_r+1);			

    // Straight Arrow Pointer (Travel Direction)
			uint8_t pointer_w = 5;              // half width of arrowhead
			uint8_t pointer_h = 9;             // full height of arrowhead
			uint8_t pointer_x = nav_x;
			uint8_t pointer_y = nav_y-nav_r-2;    //tip of arrow

			u8g2.setDrawColor(0);
			u8g2.drawBox(nav_x-(pointer_w)/2, nav_y-nav_r, pointer_w, 2);
			u8g2.setDrawColor(1);
			// arrow point    
			u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
			u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
			u8g2.drawLine(pointer_x-pointer_w-1, pointer_y+pointer_h, pointer_x-1, pointer_y);
			u8g2.drawLine(pointer_x+pointer_w+1, pointer_y+pointer_h, pointer_x+1, pointer_y);
			//arrow flats
			u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h);
			u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h);
			//arrow shaft
			u8g2.drawLine(pointer_x-pointer_w/2, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h*2);
			u8g2.drawLine(pointer_x+pointer_w/2, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h*2);


		// Waypoint Pointer
			if (gpxNav.navigating) {
				uint8_t waypoint_tip_r = 23;
				uint8_t waypoint_shaft_r = 20;    
				uint8_t waypoint_tail_r = 18;
				float waypoint_arrow_angle = 0.205;
				
				float directionToWaypoint = gpxNav.turnToActive * DEG_TO_RAD;

				int8_t waypoint_tip_x = sin(directionToWaypoint)*waypoint_tip_r+nav_x;
				int8_t waypoint_tip_y = nav_y-cos(directionToWaypoint)*waypoint_tip_r;    
				int8_t waypoint_shaft_x = sin(directionToWaypoint)*waypoint_shaft_r+nav_x;
				int8_t waypoint_shaft_y = nav_y-cos(directionToWaypoint)*waypoint_shaft_r;    

				u8g2.drawLine(nav_x+1, nav_y, waypoint_shaft_x+1, waypoint_shaft_y);
				u8g2.drawLine(nav_x, nav_y+1, waypoint_shaft_x, waypoint_shaft_y+1);
				u8g2.drawLine(nav_x, nav_y, waypoint_shaft_x, waypoint_shaft_y);          // the real center line; others are just to fatten it up
				u8g2.drawLine(nav_x-1, nav_y, waypoint_shaft_x-1, waypoint_shaft_y);
				u8g2.drawLine(nav_x, nav_y-1, waypoint_shaft_x, waypoint_shaft_y-1);

				int8_t tail_left_x = sin(directionToWaypoint - waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
				int8_t tail_left_y = nav_y - cos(directionToWaypoint - waypoint_arrow_angle) * (waypoint_tail_r);
				int8_t tail_right_x = sin(directionToWaypoint + waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
				int8_t tail_right_y = nav_y - cos(directionToWaypoint + waypoint_arrow_angle) * (waypoint_tail_r);

				u8g2.drawLine(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y);
				u8g2.drawLine(tail_right_x, tail_right_y, waypoint_tip_x, waypoint_tip_y);
				u8g2.drawLine(tail_right_x, tail_right_y, tail_left_x, tail_left_y);
				u8g2.drawTriangle(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y, tail_right_x, tail_right_y);
			}


		// Main Info ****************************************************
			uint8_t topOfFrame = 30;
			uint8_t graphBoxHeight = 40;
			uint8_t varioBarWidth = 20;
			uint8_t varioBarHeight = 101;
			uint8_t varioBoxHeight = 17;

			//blank out the bottom bit of the nav circle (to make room for climb rate and altitude and other fields etc)
			u8g2.setDrawColor(0);
			u8g2.drawBox(varioBarWidth, topOfFrame + varioBarHeight/2 + 9, 76, 2);
			u8g2.setDrawColor(1);

			// Vario Bar
			display_varioBar(topOfFrame, varioBarHeight, varioBarWidth, baro_getClimbRate());

			//air data
			display_climbRatePointerBox(varioBarWidth, topOfFrame + varioBarHeight/2 - 8, 96-varioBarWidth, varioBoxHeight, 6, baro_getClimbRate());     // x, y, w, h, triangle size, climbrate

			// alt
			display_alt_type(22, 106, leaf_8x14, DISPLAY_FIELD_ALT1, (navigatePage_cursorPosition == cursor_navigatePage_alt1));
			//display_altAboveLaunch(24, 129, baro_getAltAboveLaunch());
		
			
			// Waypoint Info
			//Name
				u8g2.setCursor(varioBarWidth + 2, topOfFrame+varioBarHeight-4);
				u8g2.setFont(u8g2_font_12x6LED_tf);
				
				if (gpxNav.navigating) u8g2.print(gpxNav.activePoint.name.c_str());
				else u8g2.print("Select Dest");

				// selection box
				if (navigatePage_cursorPosition == cursor_navigatePage_waypoint) {
					u8g2.setDrawColor(0);
					u8g2.drawRFrame(varioBarWidth + 2, topOfFrame+varioBarHeight - 17, 96 - varioBarWidth - 3, 14, 2);
					u8g2.setDrawColor(1);
					u8g2.drawRFrame(varioBarWidth+1, topOfFrame+varioBarHeight - 18, 96 - varioBarWidth - 1, 16, 3);
				}

			// Progress Bar
			  float percent_progress;
				if (gpxNav.navigating) {
					percent_progress = 1 - gpxNav.pointDistanceRemaining/gpxNav.segmentDistance;
					if (percent_progress < 0) percent_progress = 0;
				} else {
					percent_progress = 0;
				}				
				u8g2.drawFrame(0, topOfFrame+varioBarHeight-1, 96, 4);
				u8g2.drawBox(0, topOfFrame+varioBarHeight, percent_progress*96, 2);

		// User Fields ****************************************************
			uint8_t userFieldsTop = topOfFrame+varioBarHeight+3;
			uint8_t userFieldsHeight = 19;
			uint8_t userFieldsMid = userFieldsTop + userFieldsHeight;
			uint8_t userFieldsBottom = userFieldsMid + userFieldsHeight;
			uint8_t userSecondColumn = 48;

			//u8g2.drawHLine(varioBarWidth-1, userFieldsTop, 96-varioBarWidth+1);
			u8g2.drawHLine(0, userFieldsMid, 96);
			u8g2.drawHLine(0, userFieldsBottom, 96);
			u8g2.drawVLine(userSecondColumn, userFieldsTop, userFieldsHeight*2);



			display_temp(5, userFieldsMid-1, (int16_t)tempRH_getTemp());
			//display_humidity(userSecondColumn+3, userFieldsMid-1, (uint8_t)tempRH_getHumidity());

			display_distance(userSecondColumn+3, userFieldsMid-1, gpxNav.pointDistanceRemaining);

			display_accel(5, userFieldsBottom-1, IMU_getAccel());
			display_glide(userSecondColumn+3, userFieldsBottom-1, gps_getGlideRatio());


    // Footer Info ****************************************************
		
			//flight timer (display selection box if selected)
			display_flightTimer(51, 191, 0, (navigatePage_cursorPosition == cursor_navigatePage_timer));			

			// Bottom Status Icons	
				// SD Card Present
				char SDicon = 60;
				if(!SDcard_present()) SDicon = 61;
				u8g2.setCursor(12, 191);
				u8g2.setFont(leaf_icons);
				u8g2.print((char)SDicon);
			



		// Testing
		//u8g2.drawBox(8,174, 16, 16);



    
  } while ( u8g2.nextPage() ); 
  
}

void nav_cursor_move(uint8_t button) {
	if (button == UP) {
		navigatePage_cursorPosition--;
		if (navigatePage_cursorPosition < 0) navigatePage_cursorPosition = navigatePage_cursorMax;
	}
	if (button == DOWN) {
		navigatePage_cursorPosition++;
  	if (navigatePage_cursorPosition > navigatePage_cursorMax) navigatePage_cursorPosition = 0;
	}
}


void navigatePage_button(uint8_t button, uint8_t state, uint8_t count) {

	// reset cursor time out count if a button is pushed
	navigatePage_cursorTimeCount = 0;

	switch (navigatePage_cursorPosition) {
		case cursor_navigatePage_none:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) nav_cursor_move(button);     					
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
		case cursor_navigatePage_alt1:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) nav_cursor_move(button);     					
					break;
				case LEFT:
					if (DISPLAY_FIELD_ALT1 == alt_MSL && (state == PRESSED || state == HELD || state == HELD_LONG)) {
          	settings_adjustAltOffset(-1, count);
          	speaker_playSound(fx_neutral);
        	}
					break;
				case RIGHT:
					if (DISPLAY_FIELD_ALT1 == alt_MSL && (state == PRESSED || state == HELD || state == HELD_LONG)) {
          	settings_adjustAltOffset(1, count);
          	speaker_playSound(fx_neutral);
        	}
					break;
				case CENTER:
					if (state == RELEASED) settings_adjustDisplayFieldAlt1(1);
					else if (state == HELD && count == 1 && DISPLAY_FIELD_ALT1 == alt_MSL)  {
						if (settings_matchGPSAlt()) { // successful reset of AltOffset to match GPS altitude
          		speaker_playSound(fx_enter);  
              navigatePage_cursorPosition = cursor_navigatePage_none;
        		} else {                      // unsuccessful 
          	speaker_playSound(fx_cancel);
        		}
					}
					break;
			}
			break;
		case cursor_navigatePage_waypoint:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) nav_cursor_move(button);     					
					break;
				case LEFT:
					if (state == RELEASED) gpx_activatePoint(gpxNav.activePointIndex - 1);
					break;
				case RIGHT:
				  if (state == RELEASED) gpx_activatePoint(gpxNav.activePointIndex + 1);
					break;
				case CENTER:
					if (state == RELEASED) gpx_activateRoute(3);
					if (state == HELD) { 
						gpx_cancelNav();
						navigatePage_cursorPosition = cursor_navigatePage_none;
					}
					break;
			}
			break;
		/*
		case cursor_navigatePage_userField1:
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
		case cursor_navigatePage_userField2:
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
		case cursor_navigatePage_timer:
			switch(button) {
				case UP:
				case DOWN:
					if (state == RELEASED) nav_cursor_move(button);     					
					break;
				case LEFT:
					break;
				case RIGHT:
					break;
				case CENTER:
						if (state == RELEASED) {
							flightTimer_toggle();
							navigatePage_cursorPosition = cursor_navigatePage_none;
						}	else if (state == HELD) {
							flightTimer_reset();
							navigatePage_cursorPosition = cursor_navigatePage_none;
						}
						
					break;
			}
			break;
	}
}