#ifndef Leaf_SPI_h
#define Lead_SPI_h

#include <SPI.h>
#include <Arduino.h>


void setup_Leaf_SPI(void);

uint32_t baro_spiCommand(byte data);
uint16_t SPI_baro_readCalibration(unsigned char PROMaddress);
uint32_t SPI_baro_readADC(void);

#endif