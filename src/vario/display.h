/*
 * display.h
 * 
 */

#ifndef display_h
#define display_h

#include <U8g2lib.h>
#include "fonts.h"
#include "gps.h"
#include "leaf_SPI.h"
//#include "fonts.h"


#define LCD_BACKLIGHT     21  // can be used for backlight if desired (also broken out to header)


void GLCD_inst(byte data);
void GLCD_data(byte data);
void GLCD_init(void);


void display_init(void);
void display_satellites(uint16_t x, uint16_t y, uint16_t size);

void display_battery_icon(uint16_t x, uint16_t y, uint8_t battery_pct);

void display_drawTrianglePointer(uint16_t x, uint16_t y, float angle, uint16_t radius);

void display_test_bat_icon(void);
void display_test(void);
void display_test_real(void);
void display_test_real_2(void);
void display_test_real_3(void);
void display_test_big(uint8_t page);
void display_nav_page(void);
void display_thermal_page(void);



#endif