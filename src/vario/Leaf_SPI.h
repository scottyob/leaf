#ifndef Leaf_SPI_h
#define Lead_SPI_h

#include <SPI.h>
#include <Arduino.h>

// Note, ESP32 has four SPI busses.  
// SPI0 and SPI1 -> Used for internal flash  
// SPI2 -> Default bus for external peripherals (LCD, Baro, IMU).  Referred to generally as 'SPI' in code and variables below.
//      Note: On ESP32, SPI2 is called "FSPI" (sometimes called "VSPI" by some arduino references)
// SPI3 -> Used for SDcard in Quad SDIO Mode (SPI3 also called "HSPI")

// SPI and peripheral devices using the default SPI Bus (SPI2 / FSPI) 
#define SPI_SS_IMU       10
#define SPI_MOSI         11
#define SPI_CLK          12
#define SPI_MISO         13  
#define IMU_INTERRUPT    14  // INPUT
#define SPI_SS_LCD       15
#define LCD_RS           16
#define LCD_RESET        17
#define SPI_SS_BARO      18


void setup_Leaf_SPI(void);


void GLCD_spiCommand(byte data);

void imu_spiWrite(byte address, byte data);
uint8_t imu_spiRead(byte address);
uint32_t baro_spiCommand(byte data);    // TODO change to return uint8_t ?
uint16_t SPI_baro_readCalibration(unsigned char PROMaddress);
uint32_t SPI_baro_readADC(void);

#endif