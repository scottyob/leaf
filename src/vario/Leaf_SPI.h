#ifndef Leaf_SPI_h
#define Lead_SPI_h

#include <SPI.h>
#include <Arduino.h>


void setup_Leaf_SPI(void);


void GLCD_spiCommand(byte data);

void imu_spiWrite(byte address, byte data);
uint8_t imu_spiRead(byte address);
uint32_t baro_spiCommand(byte data);    // TODO change to return uint8_t ?
uint16_t SPI_baro_readCalibration(unsigned char PROMaddress);
uint32_t SPI_baro_readADC(void);

#endif