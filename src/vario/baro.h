/*
 * baro.h
 * 
 * Barometric Pressure Sensor MS5611-01BA03
 * SPI max 20MHz
 */

#ifndef baro_h
#define baro_h

#include <Arduino.h>
#include "Leaf_SPI.h"

// Sensor I2C address
#define ADDR_BARO 0x77

// Sensor commands
#define CMD_CONVERT_PRESSURE	0b01001000
#define CMD_CONVERT_TEMP		0b01011000

extern float baroAltimeterSetting;
extern  int32_t FloatAltCMinHg;
extern  int32_t FloatAltCMinHgTemp;

/* 
// Temperature Calculations
int32_t TEMP;								
int32_t TEMPfiltered;				

// Compensated Pressure Values
int32_t PRESSURE;							
int32_t PRESSUREfiltered;					

// Altitude above sea level (Pressure Altitude) in cm
int32_t P_ALT;								
int32_t P_ALTfiltered;						
int32_t P_ALTinitial;						//pressure altitude at power-on to use for autostart feature

// Climb Rate
int16_t CLIMB_RATE;							
int16_t CLIMB_RATEfiltered;					
int32_t VARIO_RATEfiltered;	
*/

void baro_adjustAltOffset(int8_t dir, uint8_t count);

// I2C Communication Functions
  uint8_t baro_sendCommand(uint8_t command);
  uint16_t baro_readCalibration(uint8_t PROMaddress);
  uint32_t baro_readADC();

// Device Management
  void baro_init(void);
	void baro_reset(void);	
	void baro_resetLaunchAlt(void);
	char baro_update(bool startNewCycle, bool doTemp);

// Device reading & data processing  
	int32_t baro_calculateAlt(void);
	void baro_filterALT(void);
	void baro_updateClimb(void);	
	void baro_filterCLIMB(void);

// Get values (mainly for display and log purposes)
	int32_t baro_getTemp(void);
	int32_t baro_getAlt (void);
	int32_t baro_getOffsetAlt(void);
	int32_t baro_getAltAtLaunch (void);
	int32_t baro_getAltAboveLaunch(void);
	int32_t baro_getAltInitial(void);
	int32_t baro_getClimbRate (void);
	int32_t baro_getVarioBar (void);

// Converstion functions
	int32_t baro_altToUnits(int32_t alt_input, bool units_feet);  
  float baro_climbToUnits(int32_t climbrate, bool units_fpm);

// Test Functions
	void baro_debugPrint(void);
	void baro_test(void);
	void baro_updateFakeNumbers(void);

#endif