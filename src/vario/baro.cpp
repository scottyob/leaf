/*
 * baro.cpp
 *
 */
#include <Arduino.h>
#include "baro.h"

// User Settings for Vario Performance
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


// Sensor Calibration Values (stored in chip PROM; must be read at startup before performing baro calculations)
uint16_t C_SENS;
uint16_t C_OFF;
uint16_t C_TCS;
uint16_t C_TCO;
uint16_t C_TREF;
uint16_t C_TEMPSENS;

// Digital read-out values
uint32_t D1_P;								//  digital pressure value (D1 in datasheet)
uint32_t D2_T;								//  digital temp value (D2 in datasheet) 
uint32_t D1_Pfiltered;
uint32_t D2_Tfiltered;

// Temperature Calculations
int32_t dT;

// Compensation Values
int64_t OFF;        // Offset at actual temperature
int64_t SENS;       // Sensitivity at actual temperature
// Extra compensation values for lower temperature ranges
int32_t TEMP2;
int64_t OFF2;
int64_t SENS2;

//Initialize the baro sensor
void baro_init(void)
{
	unsigned char i=0;
	// initialize averaging arrays
	for (i=0;i<31;i++) {
		if (i<25) varioVals[i] = 0;
		if (i<9) climbSecVals[i]=0;
		climbVals[i] = 0;
	}

	// reset baro sensor for initialization
  baro_reset();

	// read calibration values
	C_SENS = SPI_baro_readCalibration(1);
	C_OFF = SPI_baro_readCalibration(2);
	C_TCS = SPI_baro_readCalibration(3);
	C_TCO = SPI_baro_readCalibration(4);
	C_TREF = SPI_baro_readCalibration(5);
	C_TEMPSENS = SPI_baro_readCalibration(6);

	// after initialization, get first baro sensor reading to populate values
	baro_update(1);  
	delay(10);					// wait for baro sensor to process
	baro_update(2);  
	delay(10);					// wait for baro sensor to process
	baro_update(3);  
	P_ALTfiltered = P_ALT;			// filtered value should start with first reading
	lastAlt = P_ALTfiltered;		// assume we're stationary to start (previous Alt = Current ALt, so climb rate is zero)
	P_ALTinitial = P_ALTfiltered;	// also save first value to use as starting point
	baro_update(4);  
}

void baro_reset(void) {
	unsigned char command = 0b00011110;	// This is the command to reset, and for the sensor to copy calibration data into the register as needed
  baro_spiCommand(command);
	delay(3);						                // delay time required before sensor is ready
}


char baro_update(char process_step) {
  // the baro senor requires ~9ms between the command to prep the ADC and actually reading the value.
  // Since this delay is required between both pressure and temp values, we break the sensor processing 
  // up into several steps, to allow other code to process while we're waiting for the ADC to become ready.
  
	switch (process_step) {
    case 1:
      baro_spiCommand(CMD_CONVERT_PRESSURE);  // Prep baro sensor ADC to read raw pressure value (then come back for step 2 in ~10ms)      
      break;
    case 2:
      D1_P = SPI_baro_readADC();              // Read raw pressure value  
      baro_spiCommand(CMD_CONVERT_TEMP);      // Prep baro sensor ADC to read raw temperature value (then come back for step 3 in ~10ms)
      break;
    case 3:
      D2_T = SPI_baro_readADC();						  // read digital temp data
      P_ALT = baro_calculateAlt();            // calculate Pressure Altitude in cm
      break;
    case 4:
      baro_filterALT();							          // filter pressure value
	    baro_updateClimb();							        // update and filter climb rate
      baro_flightLog();                       // store any values in FlightLog as needed.  TODO: should this be every second or somewhere else?
      break;   
  }
  if(++process_step > 4) process_step = 0;  // prep for the next step in the process (if we just did step 4, we're done so set to 0.  Elsewhere, Interrupt timer will set to 1 again eventually)  
  return process_step;
}







void baro_flightLog(void) {

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
void baro_updateClimb(void)
{
	//TODO: incorporate ACCEL for added precision/accuracy
	CLIMB_RATE = (P_ALTfiltered - lastAlt) * 20;				    // climb is updated every 1/20 second, so climb rate is cm change per 1/20sec * 20

	lastAlt = P_ALTfiltered;								// store last alt value for next time
	baro_filterVARIO();									// filter vario rate and climb rate displays
	//speaker_updateClimbTone();
}


int32_t baro_calculateAlt(void)
{
	// calculate temperature (in 100ths of degrees C, from -4000 to 8500)
	  dT = D2_T - ((int32_t)C_TREF)*256;
	  TEMP = 2000 + (((int64_t)dT)*((int64_t)C_TEMPSENS))  /  pow(2,23);

  // low temperature compensation adjustments
    TEMP2 = 0;
    OFF2 = 0;
    SENS2 = 0;
    if (TEMP < 2000) {      
     TEMP2 = pow((int64_t)dT,2) / pow(2,31);
      OFF2  = 5*pow((TEMP - 2000),2) / 2;
      SENS2 = 5*pow((TEMP - 2000),2) / 4; 
    }

    // very low temperature compensation adjustments
    if (TEMP < -1500) {          
      OFF2  = OFF2 + 7 * pow((TEMP + 1500),2);
      SENS2 = SENS2 + 11 * pow((TEMP +1500),2) / 2; 
    }

    TEMP = TEMP - TEMP2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

	//Filter Temp if necessary due to noise in values
    TEMPfiltered = TEMP;    //TODO: actually filter if needed
		
  // calculate temperature compensated pressure (in 100ths of mbars)
	OFF  = (int64_t)C_OFF*pow(2,16) + (((int64_t)C_TCO) * dT)/pow(2,7);
	SENS = (int64_t)C_SENS*pow(2,15) + ((int64_t)C_TCS * dT) /pow(2,8);
	PRESSURE = ((uint64_t)D1_P * SENS / (int64_t)pow(2,21) - OFF)/pow(2,15);

	// calculate pressure altitude in cm
	return 4433100.0*(1.0-pow((float)PRESSURE/101325.0,(.190264)));
}


void baro_filterALT(void) {  
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


void baro_filterVARIO(void)
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
