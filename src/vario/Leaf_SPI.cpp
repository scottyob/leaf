#include "Leaf_SPI.h"



//#define LCD_VSPI_SS   SS      // 10-default ESP32 (LCD chip select)
//#define LCD_RS    46          // RS pin for data or instruction
#define GLCD_SS   14          // RS pin for data or instruction


#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

static const int spiClk = 1000000; 

//uninitalised pointers to SPI objects
//SPIClass * lcd_vspi = NULL;
//SPIClass * glcd_vspi = NULL;
SPIClass * baro_vspi = NULL;
SPIClass * imu_vspi = NULL;
SPIClass * glcd_vspi = NULL;


void setup_Leaf_SPI(void) {

  // Start SPI bus for peripheral devices
  baro_vspi = new SPIClass(VSPI);
  baro_vspi->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS_BARO);
  pinMode(baro_vspi->pinSS(), OUTPUT);
  digitalWrite(baro_vspi->pinSS(), HIGH);

  imu_vspi = new SPIClass(VSPI);
  imu_vspi->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS_IMU);
  pinMode(imu_vspi->pinSS(), OUTPUT);
  digitalWrite(imu_vspi->pinSS(), HIGH);

  glcd_vspi = new SPIClass(VSPI);
  glcd_vspi->begin(SPI_SCK, SPI_MISO, SPI_MOSI, GLCD_SS); //SCLK, MISO, MOSI, SS
  pinMode(glcd_vspi->pinSS(), OUTPUT);
  digitalWrite(glcd_vspi->pinSS(), HIGH);
  
}



void GLCD_spiCommand(byte data) {

  SPIClass *spi = glcd_vspi;                                       

  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));    
  digitalWrite(spi->pinSS(), LOW);                                    
  spi->transfer(data);                                                     
  digitalWrite(spi->pinSS(), HIGH);                                    
  spi->endTransaction();                                                
}



uint8_t imu_spiRead(byte address) {

  SPIClass *spi = imu_vspi;
  uint8_t val = 0;
  address |= 0x80;    // set MSB to '1' for read command
  
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(address);
  // delayMicroseconds(5);
  val = spi->transfer(0x00);  
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();

  return val;
}

void imu_spiWrite(byte address, byte data) {

  SPIClass *spi = imu_vspi;  
    
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(address);
  // delayMicroseconds(5);
  spi->transfer(data);  
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();
}


uint32_t baro_spiCommand(byte data) {

  SPIClass *spi = baro_vspi;
  uint32_t val = 0;

  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  val = spi->transfer(data);
  //Serial.println(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();

  return val;
}

// Read in sensor-specific calibration value at specified address (1-6)
uint16_t SPI_baro_readCalibration(unsigned char PROMaddress)
{

  SPIClass *spi = baro_vspi;

	uint16_t value = 0;				// This will be the final 16-bit output from the ADC
	unsigned char command = 0b10100000;	// This is the command to read from the specified address

	command += (PROMaddress << 1);		// PROM read command is 1 0 1 0 ad2 ad1 ad0 0

  
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  
  spi->transfer(command);
  value |= (uint16_t)spi->transfer(0b00000000) << 8;
  value |= (uint16_t)spi->transfer(0b00000000);

  //Serial.println(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();

 // Serial.print("Cal Val:");
  //Serial.print(value);
  //Serial.print("  ");

	return value;
}





// Read the pre-converted Digital value (either pressure or temperature, depending what 'convert' command was most recently called)
uint32_t SPI_baro_readADC(void)
{

  SPIClass *spi = baro_vspi;

	uint32_t valueL = 0;					// This will be the final 24-bit output from the ADC
  
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  
  spi->transfer(0b00000000);
  valueL |= (uint32_t)spi->transfer(0b00000000) << 16;
  valueL |= (uint32_t)spi->transfer(0b00000000) << 8;
  valueL |= (uint32_t)spi->transfer(0b00000000);

  //Serial.println(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();

 // Serial.print("ADC Val:");
 // Serial.print(valueL);
 // Serial.print("  ");

  return valueL;
}












