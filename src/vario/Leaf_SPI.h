#ifndef Leaf_SPI_h
#define Lead_SPI_h

#include <SPI.h>
#include <Arduino.h>

// Note, ESP32 has four SPI busses.  SPI0 and SPI1 are used for internal flash etc.  Default SPI bus for external peripherals is SPI2.
// On ESP32, SPI2 is referred to as FSPI (also sometimes called "VSPI" by some arduino references)
// Final note for reference: SPI3, or "HSPI" used for SD card

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