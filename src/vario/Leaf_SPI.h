#ifndef Leaf_SPI_h
#define Lead_SPI_h

#include <SPI.h>
#include <Arduino.h>


void setup_Leaf_SPI(void);

void LCD_spiCommand(byte data);

uint32_t PRESSURE_spiCommand(byte data);
uint16_t SPI_pressure_readCalibration(unsigned char PROMaddress);
uint32_t SPI_pressure_readADC(void);

#endif