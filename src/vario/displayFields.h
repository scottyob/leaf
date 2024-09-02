#ifndef displayFields_h
#define displayFields_h


#include <Arduino.h>

		
		
		
void display_clockTime(uint8_t x, uint8_t y, bool show_ampm);
void display_flightTimer(uint8_t x, uint8_t y, bool shortstring, bool selected);
uint8_t display_speed(uint8_t cursor_x, uint8_t cursor_y);
uint8_t display_speed(uint8_t x, uint8_t y, const uint8_t *font);
uint8_t display_speed(uint8_t x, uint8_t y, const uint8_t *font, bool units);
void display_heading(uint8_t cursor_x, uint8_t cursor_y, bool degSymbol);
void display_headingTurn(uint8_t cursor_x, uint8_t cursor_y);

void display_alt(uint8_t cursor_x, uint8_t cursor_y, const uint8_t *font, int32_t displayAlt);
void display_altAboveLaunch(uint8_t x, uint8_t y, int32_t aboveLaunchAlt);
void display_alt_type(uint8_t cursor_x, uint8_t cursor_y, const uint8_t * font, uint8_t altType, bool selected);
enum altTypes { 
  alt_MSL,
	alt_GPS,	
	alt_aboveLaunch,
	alt_aboveLZ,
	alt_aboveWaypoint,
	alt_AGL
};

void display_varioBar(uint8_t varioBarFrame_top, uint8_t varioBarFrame_length, uint8_t varioBarFrame_width, int32_t displayBarClimbRate);
void display_climbRatePointerBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t triSize, int16_t displayClimbRate);


void display_accel(uint8_t x, uint8_t y, float accel);
void display_glide(uint8_t x, uint8_t y, float glide);
void display_temp(uint8_t x, uint8_t y, int16_t temperature);
void display_humidity(uint8_t x, uint8_t y, uint8_t temperature);

void display_battIcon(uint8_t x, uint8_t y, bool vertical);
void display_batt_charging_fullscreen();



#endif