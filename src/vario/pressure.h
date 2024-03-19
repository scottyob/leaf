/*
 * pressure.h
 *
 *  Created on: May 5, 2012
 *      Author: oxothnk
 */

#ifndef pressure_h
#define pressure_h


/*
#include <stdint.h>
#include <math.h>
#include "SPI.h"
#include "LCD.h"
#include "settings.h"
*/

#include "Leaf_SPI.h"

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

void pressure_reset(void);
void pressure_update1(void);
void pressure_update2(void);

void pressure_write(unsigned char hexwrite);
void pressure_init(void);
uint32_t pressure_readADC(void);


void pressure_convertPressure(unsigned char cmd);
void pressure_convertTemperature(unsigned char cmd);


void pressure_update3(void);
void pressure_update4(void);
int32_t pressure_calculateAlt(void);

void pressure_updateClimb(void);

void pressure_filterALT(void);
void pressure_filterVARIO(void);

#endif /* PRESSURE_H_ */
