#include "menu_page.h"

void MenuPage::cursor_prev() {
  cursor_position--;
  if (cursor_position < 0) cursor_position = cursor_max;
}

void MenuPage::cursor_next() {
  cursor_position++;
  if (cursor_position > cursor_max) cursor_position = 0;
}
