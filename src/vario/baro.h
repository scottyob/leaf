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

// Sensor commands
#define CMD_CONVERT_PRESSURE	0b01001000
#define CMD_CONVERT_TEMP		0b01011000



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


void baro_reset(void);
char baro_update(char process_step, uint8_t Counter);

void baro_write(unsigned char hexwrite);
void baro_init(void);
uint32_t baro_readADC(void);


void baro_convertPressure(unsigned char cmd);
void baro_convertTemperature(unsigned char cmd);
int32_t baro_calculateAlt(void);
void baro_filterALT(void);
void baro_updateClimb(void);
void baro_debugPrint(void);
void baro_filterCLIMB(void);

void baro_test(void);

int32_t baro_getTemp(void);

int32_t baro_getAlt (void);
int32_t baro_getAltAtLaunch (void);
int32_t baro_getAltAboveLaunch(void);
void baro_resetLaunchAlt(void);

int32_t baro_getClimbRate (void);
int32_t baro_getVarioBar (void);



void baro_updateFakeNumbers(void);

#endif