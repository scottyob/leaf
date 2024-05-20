/*
 * gps.cpp
 *
 *
 */
#include <Arduino.h>

#include "gps.h"

const char enableGGA [] PROGMEM = "$PAIR062,0,1";   // enable GGA message every 1 second
const char enableGSV [] PROGMEM = "PAIR062,3,4";    // enable GSV message every 1 second
const char enableRMC [] PROGMEM = "PAIR062,4,1";    // enable RMC message every 1 second

const char disableGLL [] PROGMEM = "$PAIR062,1,0";  // disable message
const char disableGSA [] PROGMEM = "$PAIR062,2,0";  // disable message
const char disableVTG [] PROGMEM = "$PAIR062,5,0";  // disable message

// Setup GPS
#define gpsPort Serial0         // This is the hardware communication port (UART0) for GPS Rx and Tx lines.  We use the default ESP32S3 pins so no need to set them specifically
#define GPSBaud 115200
//#define GPSSerialBufferSize 2048
TinyGPSPlus gps;                // The TinyGPSPlus object (this is the software class that stores all the GPS info and functions)

// Satellite tracking
struct gps_sat_info sats[MAX_SATELLITES];          // GLOBAL GPS satellite info
TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, the first element is how many GSV messages (N) total will be sent
TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element is the message number (x of N)
TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element is how many satellites in view
// Fields for capturing the information from GSV strings (each GSV sentence will have info for at most 4 satellites)
TinyGPSCustom satNumber[4]; // to be initialized later
TinyGPSCustom elevation[4]; // to be initialized later
TinyGPSCustom azimuth[4];   // to be initialized later
TinyGPSCustom snr[4];       // to be initialized later


// Enable GPS Backup Power (to save satellite data and allow faster start-ups)
// This consumes a minor amount of current from the battery
// There is a loop-back pullup resistor from the backup power output to its own ENABLE line, so once backup is turned on, it will stay on even if the main processor is shut down.  
// Typically, the backup power is only turned off to enable a full cold reboot/reset of the GPS module.

void gps_setBackupPower(bool backup_power_on) {
  if (backup_power_on) digitalWrite(GPS_BACKUP_EN, HIGH);
  else digitalWrite(GPS_BACKUP_EN, LOW);
}

// Fully reset GPS using hardware signals, including powering off backup_power to erase everything
void gps_hardReset(void) {
  gps_setBackupPower(false);
  gps_softReset();
  gps_setBackupPower(true);
}

// A soft reset, keeping backup_power enabled so as not to lose saved satellite data
void gps_softReset(void) {
  digitalWrite(GPS_RESET, LOW);
  delay(100);
  digitalWrite(GPS_RESET, HIGH);  
}

void gps_enterBackupMode(void) {
  gps_setBackupPower(true); // ensure backup power enabled (should already be on)
  // TODO: send $PAIR650,0*25 command to shutdown
  // GPS should now draw ~35uA
  // cut main VCC power to further reduce power consumption to ~13uA (i.e., when whole system is shut down)
}

void gps_init(void) {

  // Set pins
  pinMode(GPS_BACKUP_EN, OUTPUT);
  gps_setBackupPower(true);       // by default, enable backup power
  pinMode(GPS_RESET, OUTPUT);
  digitalWrite(GPS_RESET, LOW);   // 
  delay(100);
  digitalWrite(GPS_RESET, HIGH);  // 




  gpsPort.begin(GPSBaud); 
  //gpsPort.setRxBufferSize(GPSSerialBufferSize);
  // Initialize all the uninitialized TinyGPSCustom objects
  for (int i=0; i<4; ++i) {
    satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
    elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
    azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
    snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
  }


  //Serial.println("Setting GPS messages");	

/*
  gps.send_P( &gpsPort, (const __FlashStringHelper *) enableGGA );
  delay(50);
  gps.send_P( &gpsPort, (const __FlashStringHelper *) enableGSV );
  delay(50);
  gps.send_P( &gpsPort, (const __FlashStringHelper *) enableRMC );
  delay(50);
  gps.send_P( &gpsPort, (const __FlashStringHelper *) disableGLL );
  delay(50);
  gps.send_P( &gpsPort, (const __FlashStringHelper *) disableGSA );
  delay(50);
  gps.send_P( &gpsPort, (const __FlashStringHelper *) disableVTG );
  delay(50);
*/


/*
  gpsPort.println("$PAIR062,0,1*3F");	//turn on GGA at 1 sec 
  gpsPort.println("$PAIR062,1,0*3F");	//turn off GLL
  gpsPort.println("$PAIR062,2,0*3C");	//turn off GSA
  gpsPort.println("$PAIR062,3,4*39");	//turn on GSV at 4 sec (up to 3 sentences)  //was 00,01*23"
  gpsPort.println("$PAIR062,4,1*3B");	//turn on RMC at 1 sec
  gpsPort.println("$PAIR062,5,0*3B");	//turn off VTG
*/

/*  
0 = NMEA_SEN_GGA    $PAIR062,0,0*3E   $PAIR062,0,1*3F
1 = NMEA_SEN_GLL    $PAIR062,1,0*3F   $PAIR062,1,1*3E
2 = NMEA_SEN_GSA    $PAIR062,2,0*3C   $PAIR062,2,1*3D
3 = NMEA_SEN_GSV    $PAIR062,3,0*3D   $PAIR062,3,1*3C   $PAIR062,3,4*39 
4 = NMEA_SEN_RMC    $PAIR062,4,0*3A   $PAIR062,4,1*3B
5 = NMEA_SEN_VTG    $PAIR062,5,0*3B   $PAIR062,5,1*3A
6 = NMEA_SEN_ZDA    $PAIR062,6,0*38   $PAIR062,6,1*39
7 = NMEA_SEN_GRS    $PAIR062,7,0*39   $PAIR062,7,1*38
8 = NMEA_SEN_GST    $PAIR062,8,0*36   $PAIR062,8,1*37
9 = NMEA_SEN_GNS    $PAIR062,9,0*37   $PAIR062,9,1*36

*/
} // gps_init


void gps_test(void) {
// Rebroadcast GPS serial data to debugger port
  while (gpsPort.available() > 0) {
    Serial.print(char(gpsPort.read()));    
  }

  /*
  while (gps.available( gpsPort )) {
    fix = gps.read();

    gps_displaySatellitesInView();
  }
  */
}


void gps_test_sats(void) {
  // Timing test to see if serial buffer size increase works
  delay(0);


  // Dispatch incoming characters
  while (gpsPort.available() > 0) {
    gps.encode(gpsPort.read());
    if (totalGPGSVMessages.isUpdated()) {
      for (int i=0; i<4; ++i) {
        int no = atoi(satNumber[i].value());
        // Serial.print(F("SatNumber is ")); Serial.println(no);
        if (no >= 1 && no <= MAX_SATELLITES) {
          sats[no-1].elevation = atoi(elevation[i].value());
          sats[no-1].azimuth = atoi(azimuth[i].value());
          sats[no-1].snr = atoi(snr[i].value());
          sats[no-1].active = true;
        }
      }
      
      int totalMessages = atoi(totalGPGSVMessages.value());
      int currentMessage = atoi(messageNumber.value());
      if (totalMessages == currentMessage) {

        // Print Sat Info
        Serial.print(F("Sats=")); Serial.print(gps.satellites.value());
        Serial.print(F(" Nums="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(i+1);
            Serial.print(F(" "));
          }
        Serial.print(F(" Elevation="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].elevation);
            Serial.print(F(" "));
          }
        Serial.print(F(" Azimuth="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].azimuth);
            Serial.print(F(" "));
          }
        
        Serial.print(F(" SNR="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].snr);
            Serial.print(F(" "));
          }
        Serial.println();

        //display_satellites(0,3,63);
        display_satellites(0,100,63);

        // Reset Active
        for (int i=0; i<MAX_SATELLITES; ++i)
          sats[i].active = false;
      }      
    }
  }
}



