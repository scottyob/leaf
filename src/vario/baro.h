/*
 * baro.h
 * 
 * Barometric Pressure Sensor MS5611-01BA03
 * SPI max 20MHz
 */

#ifndef baro_h
#define baro_h

/*
#include <stdint.h>
#include <math.h>
#include "SPI.h"
#include "LCD.h"
#include "settings.h"
*/

#include <Arduino.h>
#include "Leaf_SPI.h"
//#include "LinearRegression.h"


// Sensor commands
#define CMD_CONVERT_PRESSURE	0b01001000
#define CMD_CONVERT_TEMP		0b01011000

// Sensor Data filter values
#define PfilterSize		6			// pressure alt filter values (minimum 1, max 10)



// Temperature Calculations
extern int32_t TEMP;								// GLOBAL
extern int32_t TEMPfiltered;						// GLOBAL
// Compensated Pressure Values
extern int32_t PRESSURE;							// GLOBAL
extern int32_t PRESSUREfiltered;					// GLOBAL

// Altitude above sea level (Pressure Altitude) in cm
extern int32_t P_ALT;								// GLOBAL
extern int32_t P_ALTfiltered;						// GLOBAL
extern int32_t P_ALTinitial;						// GLOBAL	- pressure altitude at power-on to use for autostart feature


// Climb Rate
extern int16_t CLIMB_RATE;							// GLOBAL
extern int16_t CLIMB_RATEfiltered;					// GLOBAL
extern int32_t VARIO_RATEfiltered;					// GLOBAL

void baro_reset(void);
char baro_update(char process_step);

void baro_write(unsigned char hexwrite);
void baro_init(void);
uint32_t baro_readADC(void);


void baro_convertPressure(unsigned char cmd);
void baro_convertTemperature(unsigned char cmd);

int32_t baro_calculateAlt(void);

void baro_updateClimb(void);

void baro_filterALT(void);
void baro_filterVARIO(void);

void baro_flightLog(void);  

void baro_test(void);

#endif