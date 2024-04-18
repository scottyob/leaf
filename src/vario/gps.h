/*
 * gps.h
 * 
 */

/* Notes
 PQTMPVT - quectel proprietary sentence with most(all?) of the fields we need
 or we need GGA (status, time, lat/lon, alt, sats, HDOP) and RMC (status, date, time, lat/lon, speed, heading)
  and GSV for sat info when searching

  to look into:
  RLM messages? RTCM?
  DGPS set and query status

*/

#ifndef gps_h
#define gps_h


#include <TinyGPSPlus.h>
#include "display.h"

//gps object for fix data 
extern TinyGPSPlus gps;    

// GPS Satellites
#define MAX_SATELLITES 40
struct gps_sat_info{bool active; int elevation; int azimuth; int snr;};
extern struct gps_sat_info sats[MAX_SATELLITES];



void gps_init(void);
void gps_test(void);
void gps_test_sats(void);




#endif