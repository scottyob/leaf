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



void GLCD_inst(byte data);
void GLCD_data(byte data);
void GLCD_init(void);


void display_init(void);
void display_test(void);
void display_test_big(uint8_t page);
void display_satellites(uint16_t x, uint16_t y, uint16_t size);

#endif