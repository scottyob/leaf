#include <Arduino.h>
#include "PageMenuMain.h"
#include "pages.h"

#include "buttons.h"
#include "display.h"
#include "fonts.h"
#include "settings.h"


char * labels[] = {
  "Back",
  "Vario",
  "Display",
  "Units",
  "GPS",
  "Log",
  "System"  
};

// cursor positions on the main menu
enum cursor_main_menu {
  cursor_back,
  cursor_vario,
  cursor_display,
  cursor_units,
  cursor_gps,
  cursor_log,
  cursor_system
};

// all submenu pages and confirmation dialogs
enum display_menu_pages {
  page_menu_main,
  page_menu_vario,
  page_menu_display,
  page_menu_units,  
  page_menu_gps,
  page_menu_log,
  page_menu_system,
  page_menu_resetConfirm,
};

// cursor tracking for the main menu page
  int8_t cursor_position = 0;   // 0 means nothing selected
  uint8_t cursor_max = 6;       // the number of items (0-based) in the enum list above

// tracking which meny page we're on (we might move from the main meny page into a sub-meny)
  uint8_t menu_page = page_menu_main;

void MainMenuPage::draw() {
  switch (menu_page) {
    case page_menu_main:
      draw_main_menu();
      break;
    case page_menu_units:
      unitsMenuPage.draw();
      break;
  }
}


void draw_main_menu() {
  u8g2.firstPage();
  do { 
    // Title(s) 
      u8g2.setFont(leaf_6x12);
      u8g2.setCursor(2, 12);
      u8g2.setDrawColor(1);
      u8g2.print("MAIN MENU");
      u8g2.drawHLine(0, 15, 64);

    // Menu Items
      uint8_t start_y = 29;
      uint8_t setting_name_x = 3;
      uint8_t setting_choice_x = 44;    
      uint8_t menu_items_y[] = {190, 45, 60, 75, 90, 105, 120};

      //first draw cursor selection box
      u8g2.drawRBox(setting_choice_x-2, menu_items_y[cursor_position]-14, 22, 16, 2);
      
      // then draw all the menu items
      for (int i = 0; i <= cursor_max; i++) {      
        u8g2.setCursor(setting_name_x, menu_items_y[i]);
        u8g2.print(labels[i]);
        u8g2.setCursor(setting_choice_x, menu_items_y[i]);
        if (i == cursor_position) u8g2.setDrawColor(0);
        else u8g2.setDrawColor(1);
        if (cursor_position == cursor_back)
          u8g2.print((char)124);
        else
          u8g2.print((char)126);
        u8g2.setDrawColor(1);
      } 
  } while ( u8g2.nextPage() ); 
}


void cursor_prev() {
  cursor_position--;
  if (cursor_position < 0) cursor_position = cursor_max;
}

void cursor_next() {
cursor_position++;
  if (cursor_position > cursor_max) cursor_position = 0;
}


void menu_item_action(int8_t dir) {
  switch (cursor_position) {    
    case cursor_back:
      
      break;
    case cursor_vario:
      
      break;
    case cursor_display:
      
      break;
    case cursor_units:
      
      break;
    case cursor_gps:
      
      break;
    case cursor_log:
      
      break;
    case cursor_system:      
      
      break;
  }
}

bool mainMenuButtonEvent(uint8_t button, uint8_t state, uint8_t count) {
  bool redraw = false; // only redraw screen if a UI input changes something
  switch (button) {
    case UP:
      if (state == RELEASED) {
        cursor_prev();
        redraw = true;
      }
      break;
    case DOWN:
      if (state == RELEASED) {
        cursor_next();
        redraw = true;
      }
      break;
    case LEFT:
      if (state == RELEASED && cursor_position == cursor_back) {
        menu_item_action(button);
        redraw = true;
      }
      break;
    case RIGHT:
      if (state == RELEASED) {
        //go to that page
        redraw = true;
      }
      break;
    case CENTER:
      if (state == RELEASED) {
        // go to that page
        redraw = true;
      }
      break;    
  }    
  return redraw;   //update display after button push so that the UI reflects any changes immediately
}


bool MainMenuPage::button_event(uint8_t button, uint8_t state, uint8_t count) {    
  bool redraw = false; // only redraw screen if a UI input changes something
  switch (menu_page) {
    case page_menu_main:
      redraw = mainMenuButtonEvent(button, state, count);
      break;    
    case page_menu_vario:
      redraw = varioMenuPage.button_event(button, state, count);
      break;
    case page_menu_display:
      redraw = displayMenuPage.button_event(button, state, count);
      break;
    case page_menu_units:
      redraw = unitsMenuPage.button_event(button, state, count);
      break;
    case page_menu_gps:
      redraw = gpsMenuPage.button_event(button, state, count);
      break;
    case page_menu_log:
      redraw = logMenuPage.button_event(button, state, count);
      break;
    case page_menu_system:
      redraw = systemMenuPage.button_event(button, state, count);
      break;
  }
  return redraw;
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