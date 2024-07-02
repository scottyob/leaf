/*
 * baro.cpp
 *
 */
#include "baro.h"
#include "speaker.h"
#include "LinearRegression.h"



// Filter values to average/smooth out the baro sensor

  // User Settings for Vario
  #define FILTER_VALS_MAX           20                    // total array size max; for both altitude and climb 
  #define ALTITUDE_FILTER_VALS_PREF 20                    // user setting for how many altitude samples to filter over, from 1 to 20
  #define CLIMB_FILTER_VALS_PREF    20                    // how many climb rate values to average over 
  int32_t altitudeFilterVals[FILTER_VALS_MAX+1]; // use [0] as the index / bookmark
  int16_t climbFilterVals[FILTER_VALS_MAX+1];       // use [0] as the index / bookmark
  
  // LinearRegression to average out noisy sensor readings
  LinearRegression<ALTITUDE_FILTER_VALS_PREF> alt_lr;
  
  // probably gonna delete these
    #define VARIO_SENSITIVITY 3 // not sure what this is yet :)
    #define CLIMB_AVERAGE 1
    #define PfilterSize		6			// pressure alt filter values (minimum 1, max 10)
    int16_t varioVals[25];
    int16_t climbVals[31];
    int16_t climbSecVals[9];



// Baro Values
  int16_t CLIMB_RATE = 0;
  int16_t CLIMB_RATEfiltered = 0;
  
  
  int32_t VARIO_RATEfiltered = 0;

  int32_t TEMP = 0;
  int32_t TEMPfiltered = 0;

  int32_t PRESSURE = 0;
  int32_t PRESSUREfiltered = 0;

  int32_t P_ALT = 0;
  int32_t P_ALTfiltered = 0;
  int32_t P_ALTregression = 0;
  int32_t P_ALTinitial = 0;
  int32_t lastAlt = 0;

// Sensor Calibration Values (stored in chip PROM; must be read at startup before performing baro calculations)
  uint16_t C_SENS;
  uint16_t C_OFF;
  uint16_t C_TCS;
  uint16_t C_TCO;
  uint16_t C_TREF;
  uint16_t C_TEMPSENS;

// Digital read-out values
  uint32_t D1_P;								//  digital pressure value (D1 in datasheet)
  uint32_t D1_Plast = 1000;     //  save previous value to use if we ever get a mis-read from the baro sensor (initialize with a non zero starter value)
  uint32_t D2_T;								//  digital temp value (D2 in datasheet) 
  uint32_t D2_Tlast = 1000;     //  save previous value to use if we ever get a mis-read from the baro sensor (initialize with a non zero starter value)
  uint32_t D1_Pfiltered;
  uint32_t D2_Tfiltered;

// Temperature Calculations
  int32_t dT;

// Compensation Values
  int64_t OFF1;        // Offset at actual temperature
  int64_t SENS1;       // Sensitivity at actual temperature
  // Extra compensation values for lower temperature ranges
  int32_t TEMP2;
  int64_t OFF2;
  int64_t SENS2;



//*******************************************
// fake stuff for testing

int32_t fakeAlt = 0;
int16_t fakeClimbRate = 0;
int16_t fakeVarioRate = 0;
int32_t change = 1;


int32_t baro_getAlt(void) {  
  return P_ALTfiltered;  
}

// actual climb rate, for display on screen numerically, and saving in flight log
int16_t baro_getClimbRate(void) {
  //return fakeClimbRate;
  return CLIMB_RATEfiltered;
}

// climb rate for vario var visuals and perhaps sound.  This is separate in case we want to average/filter it differently
int16_t baro_getVarioBar(void) {
  return fakeVarioRate;
}

void baro_updateFakeNumbers(void) {
  fakeAlt = (float)(100 * change);
  change *= 10;
  if (change >= 1000000) {
    change = -1;
  } else if (change <= -100000) {
    change = 1;
  }
}


//143 test values
int32_t altitude_values[] = {
  1,  // index pointer to value place
5225	,
5402	,
5528	,
5605	,
5631	,
5609	,
5540	,
5426	,
5270	,
5076	,
4849	,
4593	,
4316	,
4022	,
3720	,
3418	,
3123	,
2843	,
2589	,
2368	,
2190	,
2063	,
1997	,
1999	,
2075	,
2234	,
2480	,
2816	,
3246	,
3771	,
4389	,
5098	,
5893	,
6767	,
7710	,
8712	,
9760	,
10838	,
11930	,
13018	,
14082	,
15103	,
16062	,
16940	,
17717	,
18378	,
18910	,
19301	,
19544	,
19636	,
19580	,
19383	,
19057	,
18621	,
18097	,
17516	,
16910	,
16314	,
15769	,
15312	,
14983	,
14818	,
14848	,
15096	,
15579	,
16302	,
17259	,
18431	,
19785	,
21277	,
22851	,
24442	,
25978	,
27385	,
28594	,
29540	,
30173	,
30462	,
30399	,
30000	,
29311	,
28406	,
27384	,
26362	,
25467	,
24827	,
24556	,
24744	,
25437	,
26636	,
28284	,
30267	,
32420	,
34542	,
36418	,
37844	,
38658	,
38772	,
38194	,
37040	,
35532	,
33975	,
32714	,
32076	,
32307	,
33505	,
35580	,
38246	,
41050	,
43461	,
44982	,
45287	,
44340	,
42449	,
40235	,
38491	,
37955	,
39050	,
41680	,
45171	,
48434	,
50339	,
50187	,
48114	,
45176	,
42996	,
43018	,
45674	,
49931	,
53646	,
54727	,
52582	,
48847	,
46593	,
48143	,
52937	,
57430	,
57848	,
53967	,
50113	,
51220	,
57001	,
61178	
};

// end fake stuff for testing

//Initialize the baro sensor
void baro_init(void)
{
  
  // probably don't need these
        unsigned char i=0;
      // initialize averaging arrays
        for (i=0;i<31;i++) {
          if (i<25) varioVals[i] = 0;
          if (i<9) climbSecVals[i]=0;    
          climbVals[i] = 0;
        }

	// reset baro sensor for initialization
    baro_reset();
    delay(2);

	// read calibration values
    C_SENS = spi_readBaroCalibration(1);
    C_OFF = spi_readBaroCalibration(2);
    C_TCS = spi_readBaroCalibration(3);
    C_TCO = spi_readBaroCalibration(4);
    C_TREF = spi_readBaroCalibration(5);
    C_TEMPSENS = spi_readBaroCalibration(6);

	// after initialization, get first baro sensor reading to populate values
    baro_update(1);  
    delay(10);					// wait for baro sensor to process
    baro_update(2);  
    delay(10);					// wait for baro sensor to process
    baro_update(3);  
    Serial.println(P_ALT);
    lastAlt = P_ALT;		          // assume we're stationary to start (previous Alt = Current ALt, so climb rate is zero)
    P_ALTfiltered = P_ALT;			  // filtered value should start with first reading
    P_ALTinitial = P_ALT;	        // also save first value to use as starting point (launch)
    P_ALTregression = P_ALT;

  // load the filter with our current start-up altitude
    for (int i = 1; i <= FILTER_VALS_MAX; i++) {
      altitudeFilterVals[i] = P_ALT;
      climbFilterVals[i] = 0;
    }
    // and set bookmark index to 1
    altitudeFilterVals[0] = 1;
    climbFilterVals[0] = 1;
  
  
  //fakeAlt = altitude_values[altitude_values[0]];
  //lastAlt = fakeAlt;

	baro_update(4);  
  Serial.println(P_ALT);


  //alt_lr.update((double)millis(), (double)P_ALTinitial);
}

void baro_reset(void) {
	unsigned char command = 0b00011110;	// This is the command to reset, and for the sensor to copy calibration data into the register as needed
  spi_writeBaroCommand(command);
	delay(3);						                // delay time required before sensor is ready
}


char baro_update(char process_step) {
  // the baro senor requires ~9ms between the command to prep the ADC and actually reading the value.
  // Since this delay is required between both pressure and temp values, we break the sensor processing 
  // up into several steps, to allow other code to process while we're waiting for the ADC to become ready.
  
	switch (process_step) {
    case 0:
      return process_step;                        // if baro_update is called on step 0, do nothing, and return step 0.
      break;
    case 1:
      spi_writeBaroCommand(CMD_CONVERT_PRESSURE); // Prep baro sensor ADC to read raw pressure value (then come back for step 2 in ~10ms)      
      break;
    case 2:
      D1_P = spi_readBaroADC();                   // Read raw pressure value  
      if (D1_P == 0) D1_P = D1_Plast;             // use the last value if we get a misread
      else D1_Plast = D1_P;                       // otherwise save this value for next time if needed
      spi_writeBaroCommand(CMD_CONVERT_TEMP);     // Prep baro sensor ADC to read raw temperature value (then come back for step 3 in ~10ms)
      break;
    case 3:
      D2_T = spi_readBaroADC();						        // read digital temp data
      if (D2_T == 0) D2_T = D2_Tlast;             // use the last value if we get a misread
      else D2_Tlast = D2_T;                       // otherwise save this value for next time if needed
      P_ALT = baro_calculateAlt();                // calculate Pressure Altitude in cm
      break;
    case 4:
      baro_filterALT();							              // filter pressure alt value
	    baro_updateClimb();							            // update and filter climb rate
      baro_flightLog();                           // store any values in FlightLog as needed.  TODO: should this be every second or somewhere else?
      //baro_debugPrint();
      break;   
  }
  if(++process_step > 4) process_step = 0;  // prep for the next step in the process (if we just did step 4, we're done so set to 0.  Elsewhere, Interrupt timer will set to 1 again eventually)  
  return process_step;
}


int32_t baro_calculateAlt() {
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
    OFF1 = OFF1 - OFF2;
    SENS1 = SENS1 - SENS2;

	//Filter Temp if necessary due to noise in values
    TEMPfiltered = TEMP;    //TODO: actually filter if needed
		
  // calculate temperature compensated pressure (in 100ths of mbars)
    OFF1  = (int64_t)C_OFF*pow(2,16) + (((int64_t)C_TCO) * dT)/pow(2,7);
    SENS1 = (int64_t)C_SENS*pow(2,15) + ((int64_t)C_TCS * dT) /pow(2,8);
    PRESSURE = ((uint64_t)D1_P * SENS1 / (int64_t)pow(2,21) - OFF1)/pow(2,15);

	// calculate pressure altitude in cm
	  return 4433100.0*(1.0-pow((float)PRESSURE/101325.0,(.190264)));
}

uint8_t fakeAltCounter = 0;

void baro_filterALT(void) {  

  // new way with regression:
    alt_lr.update((double)millis(), (double)P_ALT);
    LinearFit fit = alt_lr.fit();
    P_ALTregression = linear_value(&fit, (double)millis());

  
  //old way with averaging last N values equally:
    P_ALTfiltered = 0;
    int8_t filterBookmark = altitudeFilterVals[0];       // start at the saved spot in the filter array
    int8_t filterIndex = filterBookmark;                 // and create an index to track all the values we need for averaging

    altitudeFilterVals[filterBookmark] = P_ALT;          // load in the new value at the bookmarked spot
    if (++filterBookmark >= FILTER_VALS_MAX)    // increment bookmark for next time
      filterBookmark = 1;                                // wrap around the array for next time if needed
    altitudeFilterVals[0] = filterBookmark;              // and save the bookmark for next time

    // sum up all the values from this spot and previous, for the correct number of samples (user pref)
    for (int i = 0; i < ALTITUDE_FILTER_VALS_PREF; i++) {
      P_ALTfiltered += altitudeFilterVals[filterIndex];   
      filterIndex--;
      if (filterIndex <= 0) filterIndex = FILTER_VALS_MAX; // wrap around the array
    }
    P_ALTfiltered /= ALTITUDE_FILTER_VALS_PREF; // divide to get the average
  

  // temp testing stuff  
    fakeAlt = altitude_values[altitude_values[0]];
    if(++fakeAltCounter >=10) {
      fakeAltCounter = 0;
      altitude_values[0] = altitude_values[0] + 1;
      if (altitude_values[0] == 144) altitude_values[0] = 1;
      //baro_updateClimb();
    }

}



// Update Climb
void baro_updateClimb() {
	//TODO: incorporate ACCEL for added precision/accuracy
	CLIMB_RATE = (P_ALTfiltered - lastAlt) * 20;	// climb is updated every 1/20 second, so climb rate is cm change per 1/20sec * 20
  lastAlt = P_ALTfiltered;								      // store last alt value for next time

  //filter climb rate
    CLIMB_RATEfiltered = 0;
    int8_t filterBookmark = climbFilterVals[0];       // start at the saved spot in the filter array
    int8_t filterIndex = filterBookmark;              // and create an index to track all the values we need for averaging

    climbFilterVals[filterBookmark] = CLIMB_RATE;     // load in the new value at the bookmarked spot
    if (++filterBookmark >= FILTER_VALS_MAX)    // increment bookmark for next time
      filterBookmark = 1;                             // wrap around the array for next time if needed
    climbFilterVals[0] = filterBookmark;              // and save the bookmark for next time

    // sum up all the values from this spot and previous, for the correct number of samples (user pref)
    for (int i = 0; i < CLIMB_FILTER_VALS_PREF; i++) {
      CLIMB_RATEfiltered += climbFilterVals[filterIndex];   
      filterIndex--;
      if (filterIndex <= 0) filterIndex = FILTER_VALS_MAX; // wrap around the array
    }
    CLIMB_RATEfiltered /= CLIMB_FILTER_VALS_PREF; // divide to get the average
  



  //fakeClimbRate = (fakeAlt - lastAlt) / 6;      // test value changes every 2 seconds, so climbrate needs to be halved
  //lastAlt = fakeAlt;
	
	//baro_filterCLIMB();									        // filter vario rate and climb rate displays
	
  speaker_updateVarioNoteSample(CLIMB_RATEfiltered);
}

void baro_debugPrint() {
  /*
  Serial.print("D1_P:");
  Serial.print(D1_P);
  Serial.print(", D2_T:");
  Serial.print(D2_T);         //has been zero, perhaps because GPS serial buffer processing delayed the ADC prep for reading this from baro chip
  */
  Serial.print("LastAlt:");
  Serial.print(lastAlt);// - P_ALTinitial);
  Serial.print(", ALT:");
  Serial.print(P_ALT);// - P_ALTinitial);
  Serial.print(", FILTERED:");
  Serial.print(P_ALTfiltered);// - P_ALTinitial);
  Serial.print(", REGRESSED:");
  Serial.print(P_ALTregression);// - P_ALTinitial);
  //Serial.print(", TEMP:");
  //Serial.print(TEMP);
  Serial.print(", CLIMB:");
  Serial.print(CLIMB_RATE);
  Serial.print(", CLIMB_FILTERED:");
  Serial.println(CLIMB_RATEfiltered);
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




void baro_filterCLIMB(void)
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

char process_step_test = 0;

void baro_test(void) {
  delay(10);  // delay for ADC processing bewteen update steps
  process_step_test = baro_update(process_step_test);
  if (process_step_test == 0) {
    process_step_test++;  
    Serial.print("PressureAltCm:");
    Serial.print(P_ALT);
    Serial.print(",");
    Serial.print("FilteredAltCm:");
    Serial.print(P_ALTfiltered);
    Serial.print(",");
    Serial.println(P_ALTregression);
  }
}
