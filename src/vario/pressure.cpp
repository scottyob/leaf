/*
 * pressure.cpp
 *
 *  Created on: May 5, 2012 (converted to Arduino 3/17/2024)
 *      Author: oxothnk
 */

#include <Arduino.h>
#include "pressure.h"

#define VARIO_SENSITIVITY 3
#define CLIMB_AVERAGE 1

// global variables
int32_t TEMP = 0;
int32_t TEMPfiltered = 0;

int32_t PRESSURE = 0;
int32_t PRESSUREfiltered = 0;

int32_t P_ALT = 0;
int32_t P_ALTfiltered = 0;
int32_t P_ALTinitial = 0;
int32_t lastAlt = 0;

int32_t AltFilterReadings[10];

int16_t CLIMB_RATE = 0;
int16_t CLIMB_RATEfiltered = 0;
int32_t VARIO_RATEfiltered = 0;

int16_t varioVals[25];
int16_t climbVals[31];
int16_t climbSecVals[9];


// Calibration Values
uint16_t C_SENS;
uint16_t C_OFF;
uint16_t C_TCS;
uint16_t C_TCO;
uint16_t C_TREF;
uint16_t C_TEMPSENS;

// Digital read-out values
uint32_t D_P;								//  pressure
uint32_t D_T;								//  temp      //TODO: should this be signed for below freezing?  or is it K?
uint32_t D_Pfiltered;
uint32_t D_Tfiltered;

// Temperature Calculations
int32_t dT;

// Compensated Pressure Values
int64_t pOFF;
int64_t SENS;



/*            Two SPI command functions not needed with Arduino SPI implementation

// Commands Pressure Sensor to measure analog signal and place digital conversion in buffer
void pressure_convertPressure(unsigned char cmd)
{
	
  char cSREG = SREG;					// Store interrupt value
	cli();

	PRESSURE_PORT &= ~PRESSURE_CS;		// Chip Select low to activate/select Pressure Sensor
	_delay_us(1);
	SPDR = cmd;							// Convert at highest resolution (8.5ms delay before ready to read)
	loop_until_bit_is_set(SPSR, SPIF);
	cmd = SPDR;
	PRESSURE_PORT |= PRESSURE_CS;		// Bring Chip Select high to disable Pressure Sensor

	SREG = cSREG;						// return interrupts to previous condition
  
}

// Commands Pressure Sensor to measure analog signal and place digital conversion in buffer
void pressure_convertTemperature(unsigned char cmd)
{
	char cSREG = SREG;					// Store interrupt value
	cli();

	PRESSURE_PORT &= ~PRESSURE_CS;		// Chip Select low to activate/select Pressure Sensor
	_delay_us(1);
	SPDR = cmd;					// Convert at highest resolution (8.5ms delay before ready to read)
	loop_until_bit_is_set(SPSR, SPIF);
	cmd = SPDR;
	PRESSURE_PORT |= PRESSURE_CS;		// Bring Chip Select high to disable Pressure Sensor

	SREG = cSREG;						// return interrupts to previous condition
}
*/




//Initialize the pressure sensor
void pressure_init(void)
{
	unsigned char i=0;
	// initialize averaging arrays
	for (i=0;i<31;i++) {
		if (i<25) varioVals[i] = 0;
		if (i<9) climbSecVals[i]=0;
		climbVals[i] = 0;
	}

	// reset pressure sensor for initialization
	pressure_reset();

	// read calibration values
	C_SENS = SPI_pressure_readCalibration(1);
	C_OFF = SPI_pressure_readCalibration(2);
	C_TCS = SPI_pressure_readCalibration(3);
	C_TCO = SPI_pressure_readCalibration(4);
	C_TREF = SPI_pressure_readCalibration(5);
	C_TEMPSENS = SPI_pressure_readCalibration(6);

	// after initialization, get first pressure sensor reading to populate values
	delay(500);
	pressure_update1();
	delay(100);					// wait for pressure sensor to process
	pressure_update2();
	delay(100);					// wait for pressure sensor to process
	pressure_update3();
	P_ALTfiltered = P_ALT;			// filtered value should start with first reading
	lastAlt = P_ALTfiltered;		// assume we're stationary to start (zero out climb rate)
	P_ALTinitial = P_ALTfiltered;	// also save first value to use as starting point
	pressure_update4();
}

void pressure_reset(void) {
	unsigned char command = 0b00011110;	// This is the command to reset, and for the sensor to copy calibration data into the register as needed
  PRESSURE_spiCommand(command);

/*
	char cSREG = SREG;					// Store interrupt value
	cli();

	PRESSURE_PORT &= ~PRESSURE_CS;		// Chip Select low to activate/select Pressure Sensor
	_delay_us(1);

	SPDR = command;						// Send ADC read command
	loop_until_bit_is_set(SPSR, SPIF);
	command = SPDR;					// Clear SPIF bit

	PRESSURE_PORT |= PRESSURE_CS;		// Bring Chip Select high to disable Pressure Sensor

	SREG = cSREG;						// return interrupts to previous condition
*/
	delay(3);						// delay time required before sensor is ready
}


void pressure_update1(void) {
	//pressure_convertPressure(CMD_CONVERT_PRESSURE);	// convert pressure
  PRESSURE_spiCommand(CMD_CONVERT_PRESSURE);
}


void pressure_update2(void) {
	D_P = SPI_pressure_readADC();						// read digital pressure data
	//pressure_convertTemperature(CMD_CONVERT_TEMP);	// convert temperature
  PRESSURE_spiCommand(CMD_CONVERT_TEMP);
}


void pressure_update3(void)  						// currently happens every 1/8 sec
{
	D_T = SPI_pressure_readADC();						// read digital temp data

	P_ALT = pressure_calculateAlt();				// get pressure alt in cm
}


void pressure_update4(void)
{
	pressure_filterALT();							// filter pressure value
	pressure_updateClimb();							// update and filter climb rate
/*
	//save values for logbook
	if (VARIO_RATEfiltered > 0 && VARIO_RATEfiltered > logbook_CLIMB) {
		logbook_CLIMB = VARIO_RATEfiltered;
	} else if (VARIO_RATEfiltered < 0 && VARIO_RATEfiltered < logbook_SINK) {
		logbook_SINK = VARIO_RATEfiltered;
	}
	if ((P_ALTfiltered + ALT_OFFSET) > logbook_ALT_max) {
		logbook_ALT_max = (P_ALTfiltered + ALT_OFFSET);
	}
  */
}

// Update Climb
void pressure_updateClimb(void)
{
	//TODO: changed this for testing to use Accel as climb
	CLIMB_RATE = (P_ALTfiltered - lastAlt) * 8;				// climb is updated every 1/8 second, so climb rate is cm change per 1/8sec * 8

	//CLIMB_RATE = GFORCE - 800;

	lastAlt = P_ALTfiltered;								// store last alt value for next time
	pressure_filterVARIO();									// filter vario rate and climb rate displays
	//speaker_updateClimbTone();
}


int32_t pressure_calculateAlt(void)
{
	// calculate temperature
	dT = D_T - ((int32_t)C_TREF)*256;
	TEMP = 2000 + (((int64_t)dT)*((int64_t)C_TEMPSENS))  /  pow(2,23);
    TEMPfiltered = TEMP;

	//calculate filtered temp for comparison (just testing)
	// dT = D_Tfiltered - ((signed long)C_TREF)*256;
	// TEMPfiltered = 2000 + (((signed long long)dT)*((signed long long)C_TEMPSENS))  /  pow(2,23);   // real ambient temp value to display to user


	// calculate temperature compensated pressure
	pOFF = (int64_t)C_OFF*pow(2,16) + (((int64_t)C_TCO) * dT)/pow(2,7);
	SENS = (int64_t)C_SENS*pow(2,15)+((int64_t)C_TCS * dT)/pow(2,8);

	PRESSURE = ((uint64_t)D_P * SENS / (int64_t)pow(2,21) - pOFF)/pow(2,15);


	// calculate pressure altitude in cm
	return 4433100.0*(1.0-pow((float)PRESSURE/101325.0,(.190264)));
}


void pressure_filterALT(void)
{
  
  for(int i=9; i>0; i--) {
    AltFilterReadings[i] = AltFilterReadings[i-1]; // move every reading down one
  }

  AltFilterReadings[0] = P_ALT;                   // Put new reading at the head of the array

  P_ALTfiltered = 0;

  for (int i = 0; i<PfilterSize; i++) {
  P_ALTfiltered += AltFilterReadings[i];
  }

  P_ALTfiltered /= PfilterSize;

	//P_ALTfiltered = (P_ALTfiltered * (PfilterSize - 1) + P_ALT) / PfilterSize;   	// filter by weighting old values higher
}


void pressure_filterVARIO(void)
{
	uint32_t sum = 0;
	unsigned char i = 0;

	//add new value to vario values
	varioVals[24]++;
	if (varioVals[24] > 4*VARIO_SENSITIVITY ||
		varioVals[24] >= 24) {
		varioVals[24] = 0;
	}
	varioVals[varioVals[24]] = CLIMB_RATE;


	for (i=0; i<4*VARIO_SENSITIVITY;i++) {
		sum += varioVals[i];
		//if (i>=24) break;						// just a safety check in case VARIO_SENS.. got set too high
	}
	VARIO_RATEfiltered = sum/(4*VARIO_SENSITIVITY);

	if (CLIMB_AVERAGE == 0) {
		CLIMB_RATEfiltered = VARIO_RATEfiltered;
	} else {
		// filter Climb Rate over 1 second...
		climbSecVals[8]++;
		if (climbSecVals[8] >= 8) {climbSecVals[8] = 0;}
		climbSecVals[climbSecVals[8]] = CLIMB_RATE;

		// ...and then every second, average over 0-30 seconds for the numerical display
		if (climbSecVals[8] == 0) {
			sum = 0;
			for (i=0; i<8;i++) {
				sum += climbSecVals[i];
			}

			climbVals[30]++;
			if (climbVals[30] >= CLIMB_AVERAGE*10) {climbVals[30] = 0;}
			climbVals[climbVals[30]] = sum/8;

			sum = 0;
			for (i=0; i<CLIMB_AVERAGE*10;i++) {
				sum += climbVals[i];
			}
			CLIMB_RATEfiltered = sum/(CLIMB_AVERAGE*10);
		}
	}
/*
	// vario average setting is stored in 1/2 seconds, but vario samples come in every 1/8th second,
	// so multiply [1/2 sec] setting by 4, to get number of [1/8th rate] samples.
	VARIO_RATEfiltered = (VARIO_RATEfiltered * (4*VARIO_SENSITIVITY - 1) + CLIMB_RATE) / (4*VARIO_SENSITIVITY);  // filter by weighting old values higher
*/
}
