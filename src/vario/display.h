/*
 * display.h
 * 
 */

#ifndef display_h
#define display_h

#include <U8g2lib.h>

#define LCD_BACKLIGHT    21  // can be used for backlight if desired (also broken out to header)
#define LCD_RS           16
#define LCD_RESET        17

extern U8G2_ST7539_192X64_F_4W_HW_SPI u8g2;

void GLCD_inst(byte data);
void GLCD_data(byte data);
//void GLCD_init(void);

// keep track of pages
enum display_page_actions {
  page_home,    // go to home screen (probably thermal page)
  page_prev,    // go to page -1
  page_next,    // go to page +1
  page_back     // go to page we were just on before (i.e. step back in a menu tree, or cancel a dialog page back to previous page)
};

enum display_main_pages {  
  page_sats,
  page_thermal,
  page_nav,
  page_menu,   
  page_last,
  page_charging  
};

void display_turnPage(uint8_t action);
void display_setPage(uint8_t targetPage);
uint8_t display_getPage(void);

void display_init(void);
void display_update(void);
void display_clear(void);

void display_battery_icon(uint16_t x, uint16_t y, uint8_t battery_pct);

// make these accessible so we can show these fields in other pages
  void display_alt(uint8_t cursor_x, uint8_t cursor_y, const uint8_t *font, int32_t displayAlt);
  void display_flightTimer(uint8_t x, uint8_t y, bool shortstring);
  void display_clockTime(uint8_t x, uint8_t y);
//

void display_drawTrianglePointer(uint16_t x, uint16_t y, float angle, uint16_t radius);

void display_test_bat_icon(void);
void display_test(void);
void display_test_real(void);
void display_test_real_2(void);
void display_test_real_3(void);
void display_test_big(uint8_t page);


void display_page_satellites(void);
void display_page_nav(void);
void display_page_thermal(void);
void display_page_charging(void);
void display_page_menu(void);



#endif