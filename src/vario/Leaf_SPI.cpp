#include "Leaf_SPI.h"


// SPI STUFF
#define VSPI_MISO MISO        // 13-default ESP32
#define VSPI_MOSI MOSI        // 11-default ESP32
#define VSPI_SCLK SCK         // 12-default ESP32
#define PRESSURE_VSPI_SS  47  // Pressure chip select
#define LCD_VSPI_SS   SS      // 10-default ESP32 (LCD chip select)
//#define LCD_RS    46          // RS pin for data or instruction


#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * lcd_vspi = NULL;
SPIClass * pressure_vspi = NULL;


void setup_Leaf_SPI(void) {

// Start SPI bus for peripheral devices
pressure_vspi = new SPIClass(VSPI);
pressure_vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, PRESSURE_VSPI_SS);
pinMode(pressure_vspi->pinSS(), OUTPUT);

lcd_vspi = new SPIClass(VSPI);
lcd_vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, LCD_VSPI_SS); //SCLK, MISO, MOSI, SS
pinMode(lcd_vspi->pinSS(), OUTPUT); //VSPI SS
//pinMode(LCD_RS, OUTPUT);

}

void LCD_spiCommand(byte data) {

  SPIClass *spi = lcd_vspi;

  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(data);
  //Serial.println(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();
}

uint32_t PRESSURE_spiCommand(byte data) {

  SPIClass *spi = pressure_vspi;
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
uint16_t SPI_pressure_readCalibration(unsigned char PROMaddress)
{

  SPIClass *spi = pressure_vspi;

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
uint32_t SPI_pressure_readADC(void)
{

  SPIClass *spi = pressure_vspi;

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












