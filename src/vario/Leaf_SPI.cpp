
#include <SPI.h>
#include "Leaf_SPI.h"

static const int spiClk = 1000000; 

void spi_init(void) {
  pinMode(SPI_SS_IMU, OUTPUT);
  pinMode(SPI_SS_LCD, OUTPUT);
  pinMode(SPI_SS_BARO, OUTPUT);

  digitalWrite(SPI_SS_IMU, HIGH);
  digitalWrite(SPI_SS_LCD, HIGH);
  digitalWrite(SPI_SS_BARO, HIGH);

  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_SS_BARO);
}


void spi_writeGLCD(byte data) {

  Serial.print("starting GLCD write. ");
  
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  Serial.print("Began tx. ");
  digitalWrite(SPI_SS_LCD, LOW);
  Serial.print("SS low. ");
  SPI.transfer(data);     
  Serial.print("sent data. ");
  digitalWrite(SPI_SS_LCD, HIGH);
  Serial.print("SS high. ");
  SPI.endTransaction();
  Serial.println("end tx. ");
}



uint8_t spi_readIMUByte(byte address) {
  
  uint8_t val = 0;
  address |= 0x80;    // set MSB to '1' for read command
  
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_SS_IMU, LOW); //pull SS slow to prep other end for transfer
  SPI.transfer(address);  
  val = SPI.transfer(0x00);  
  digitalWrite(SPI_SS_IMU, HIGH); //pull ss high to signify end of data transfer
  SPI.endTransaction();

  return val;
}

void spi_writeIMUByte(byte address, byte data) {

  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_SS_IMU, LOW); //pull SS slow to prep other end for transfer
  SPI.transfer(address);
  // delayMicroseconds(5);
  SPI.transfer(data);  
  digitalWrite(SPI_SS_IMU, HIGH); //pull ss high to signify end of data transfer
  SPI.endTransaction();
}


uint32_t spi_writeBaroCommand(byte data) {
  
  uint32_t val = 0;

  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_SS_BARO, LOW);
  val = SPI.transfer(data);     
  digitalWrite(SPI_SS_BARO, HIGH);
  SPI.endTransaction();
  
  return val;
}

// Read in sensor-specific calibration value at specified address (1-6)
uint16_t spi_readBaroCalibration(unsigned char PROMaddress) {

	uint16_t value = 0;				// This will be the final 16-bit output from the ADC
	unsigned char command = 0b10100000;	// This is the command to read from the specified address
	command += (PROMaddress << 1);		// PROM read command is 1 0 1 0 ad2 ad1 ad0 0
  
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_SS_BARO, LOW); //pull SS slow to prep other end for transfer
  
  SPI.transfer(command);
  value |= (uint16_t)SPI.transfer(0b00000000) << 8;
  value |= (uint16_t)SPI.transfer(0b00000000);

  //Serial.println(data);
  digitalWrite(SPI_SS_BARO, HIGH); //pull ss high to signify end of data transfer
  SPI.endTransaction();

	return value;
}





// Read the pre-converted Digital value (either pressure or temperature, depending what 'convert' command was most recently called)
uint32_t spi_readBaroADC(void) {

  uint32_t valueL = 0;					// This will be the final 24-bit output from the ADC
  
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_SS_BARO, LOW); //pull SS slow to prep other end for transfer
  
  SPI.transfer(0b00000000);
  valueL |= (uint32_t)SPI.transfer(0b00000000) << 16;
  valueL |= (uint32_t)SPI.transfer(0b00000000) << 8;
  valueL |= (uint32_t)SPI.transfer(0b00000000);

  //Serial.println(data);
  digitalWrite(SPI_SS_BARO, HIGH); //pull ss high to signify end of data transfer
  SPI.endTransaction();

  return valueL;
}












