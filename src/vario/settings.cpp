

#include <Preferences.h> 
#include <nvs_flash.h>

#include "settings.h"
#include "speaker.h"

#define RW_MODE false
#define RO_MODE true


// Global Variables for Current Settings
  // Vario Settings    
    int8_t SINK_ALARM;
    int8_t VARIO_AVERAGE;
    int8_t CLIMB_AVERAGE;
    int8_t CLIMB_START;
    int8_t VOLUME_VARIO;
    int8_t LIFTY_AIR;
    int32_t ALT_OFFSET;
  // GPS & Track Log Settings
    bool DISTANCE_FLOWN;
    int8_t GPS_SETTING;
    bool TRACK_SAVE;
    bool AUTO_START;
  // System Settings
    int16_t TIME_ZONE;
    int8_t VOLUME_SYSTEM;
    int16_t CONTRAST;
    bool ENTER_BOOTLOAD;
    bool AUTO_OFF;
  // Unit Values
    bool UNITS_climb;
    bool UNITS_alt;
    bool UNITS_temp;
    bool UNITS_speed;
    bool UNITS_heading;
    bool UNITS_distance;
    bool UNITS_hours;

Preferences leafPrefs;

void settings_init() {
  
  settings_loadDefaults();  // load defaults regardless, but we'll overwrite these with saved user settings (if available)

  // Check if settings have been saved before (or not), then save defaults (or grab saved settings)
    leafPrefs.begin("varioPrefs", RO_MODE);         // open (or create if needed) the varioPrefs namespace for user settings/preferences
    bool newBootupVario = !leafPrefs.isKey("nvsInitVario");   // check if we've ever initialized the non volatile storage (nvs), or if this is a new device boot up for the first time
    // new bootup into Vario
    if (newBootupVario) {
      leafPrefs.end();
      leafPrefs.begin("varioPrefs", RW_MODE);    
      settings_save();            // save defaults to NVS0      
      leafPrefs.end();
    } else {
      settings_retrieve();
    }

}


void factoryResetVario() {
  leafPrefs.remove("nvsInitVario");   // remove this key so that we force a factory settings reload on next reboot
}


void settings_loadDefaults() {
  // Vario Settings    
    SINK_ALARM = DEF_SINK_ALARM;
    VARIO_AVERAGE = DEF_VARIO_AVERAGE;
    CLIMB_AVERAGE = DEF_CLIMB_AVERAGE;
    CLIMB_START = DEF_CLIMB_START;
    VOLUME_VARIO = DEF_VOLUME_VARIO;
    LIFTY_AIR = DEF_LIFTY_AIR;
    ALT_OFFSET = DEF_ALT_OFFSET;
  // GPS & Track Log Settings
    DISTANCE_FLOWN = DEF_DISTANCE_FLOWN;
    GPS_SETTING = DEF_GPS_SETTING;
    TRACK_SAVE = DEF_TRACK_SAVE;
    AUTO_START = DEF_AUTO_START;
  // System Settings
    TIME_ZONE = DEF_TIME_ZONE;
    VOLUME_SYSTEM = DEF_VOLUME_SYSTEM;
    CONTRAST = DEF_CONTRAST;
    ENTER_BOOTLOAD = DEF_ENTER_BOOTLOAD;
    AUTO_OFF = DEF_AUTO_OFF;
  // Unit Values
    UNITS_climb = DEF_UNITS_climb;
    UNITS_alt = DEF_UNITS_alt;
    UNITS_temp = DEF_UNITS_temp;
    UNITS_speed = DEF_UNITS_speed;
    UNITS_heading = DEF_UNITS_heading;
    UNITS_distance = DEF_UNITS_distance;
    UNITS_hours = DEF_UNITS_hours;
}

void settings_retrieve() {
  leafPrefs.begin("varioPrefs", RO_MODE);

  // Vario Settings    
    SINK_ALARM =      leafPrefs.getChar ("SINK_ALARM");
    VARIO_AVERAGE =   leafPrefs.getChar("VARIO_AVERAGE");
    CLIMB_AVERAGE =   leafPrefs.getChar("CLIMB_AVERAGE");
    CLIMB_START =     leafPrefs.getChar("CLIMB_START");
    VOLUME_VARIO =    leafPrefs.getChar("VOLUME_VARIO");
    LIFTY_AIR =       leafPrefs.getChar ("LIFTY_AIR");
    ALT_OFFSET =      leafPrefs.getLong ("ALT_OFFSET");
  // GPS & Track Log Settings
    DISTANCE_FLOWN =  leafPrefs.getBool("DISTANCE_FLOWN");
    GPS_SETTING =     leafPrefs.getChar("GPS_SETTING");
    TRACK_SAVE =      leafPrefs.getBool("TRACK_SAVE");
    AUTO_START =      leafPrefs.getBool("AUTO_START");
  // System Settings
    TIME_ZONE =       leafPrefs.getShort("TIME_ZONE");
    VOLUME_SYSTEM =   leafPrefs.getChar("VOLUME_SYSTEM");
    CONTRAST =        leafPrefs.getShort("CONTRAST");
    ENTER_BOOTLOAD =  leafPrefs.getBool("ENTER_BOOTLOAD");
    AUTO_OFF =        leafPrefs.getBool("AUTO_OFF");
  // Unit Values
    UNITS_climb =     leafPrefs.getBool("UNITS_climb");
    UNITS_alt =       leafPrefs.getBool("UNITS_alt");
    UNITS_temp =      leafPrefs.getBool("UNITS_temp");
    UNITS_speed =     leafPrefs.getBool("UNITS_speed");
    UNITS_heading =   leafPrefs.getBool("UNITS_heading");
    UNITS_distance =  leafPrefs.getBool("UNITS_distance");
    UNITS_hours =     leafPrefs.getBool("UNITS_hours");

  leafPrefs.end();
}

void settings_save() {
  // Save settings before shutdown (or other times as needed)

  leafPrefs.begin("varioPrefs", RW_MODE);

  // save flag to indicate we have previously initialized NVS storage and have saved settings available
    leafPrefs.putBool("nvsInitVario", true);

  // Vario Settings    
    leafPrefs.putChar ("SINK_ALARM", SINK_ALARM);
    leafPrefs.putChar("VARIO_AVERAGE", VARIO_AVERAGE);
    leafPrefs.putChar("CLIMB_AVERAGE", CLIMB_AVERAGE);
    leafPrefs.putChar("CLIMB_START", CLIMB_START);
    leafPrefs.putChar("VOLUME_VARIO", VOLUME_VARIO);
    leafPrefs.putChar ("LIFTY_AIR", LIFTY_AIR);
    leafPrefs.putLong ("ALT_OFFSET", ALT_OFFSET);
  // GPS & Track Log Settings
    leafPrefs.putUChar("DISTANCE_FLOWN", DISTANCE_FLOWN);
    leafPrefs.putUChar("GPS_SETTING", GPS_SETTING);
    leafPrefs.putBool("TRACK_SAVE", TRACK_SAVE);
    leafPrefs.putBool("AUTO_START", AUTO_START);
  // System Settings
    leafPrefs.putShort("TIME_ZONE", TIME_ZONE);
    leafPrefs.putChar("VOLUME_SYSTEM", VOLUME_SYSTEM);
    leafPrefs.putShort("CONTRAST", CONTRAST);
    leafPrefs.putBool("ENTER_BOOTLOAD", ENTER_BOOTLOAD);
    leafPrefs.putBool("AUTO_OFF", AUTO_OFF);
  // Unit Values
    leafPrefs.putBool("UNITS_climb", UNITS_climb);
    leafPrefs.putBool("UNITS_alt", UNITS_alt);
    leafPrefs.putBool("UNITS_temp", UNITS_temp);
    leafPrefs.putBool("UNITS_speed", UNITS_speed);
    leafPrefs.putBool("UNITS_heading", UNITS_heading);
    leafPrefs.putBool("UNITS_distance", UNITS_distance);
    leafPrefs.putBool("UNITS_hours", UNITS_hours);

  leafPrefs.end();
}


void settings_reset() {
  settings_loadDefaults();
  settings_save();
}



// we probably should never have to call this
void settings_totallyEraseNVS() {
  nvs_flash_erase();      // erase the NVS partition and...
  nvs_flash_init();       // initialize the NVS partition.  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adjust individual settings

void settings_adjustContrast(uint16_t val) {
  if (val == -1) {
    // TODO: set contrast lower
  } else if (val == 1) {
    // TODO: set contrast higher
  } else if (val == 0) {
    // TODO: set contrast to default
  } else if (val <= CONTRAST_MAX && val >= CONTRAST_MIN) {
    // TODO: set contrast to val
  }
}

 


void settings_setAltOffset(int32_t value) {
  ALT_OFFSET = value;
}

void settings_adjustAltOffset(int8_t dir, uint8_t count) {
	int increase = 100;                 // 1m increments (100cm)
	if (UNITS_alt == 1) increase = 30;  // 1 ft increments (30cm)
	if (count >= 40) increase *= 5;
	if (dir >= 1) ALT_OFFSET += increase;
	else if (dir <= -1) ALT_OFFSET -= increase;	
}

void settings_adjustSinkAlarm(int8_t dir) {
	uint16_t * sound = fx_neutral;

  if (dir > 0) {
    sound = fx_increase;
    if (++SINK_ALARM > 0) {      
      SINK_ALARM = SINK_ALARM_MAX;      // if we were at 0 and now are at positive 1, go back to max sink rate
    } else if (SINK_ALARM > SINK_ALARM_MIN) {
      sound = fx_cancel;
      SINK_ALARM = 0; // if we were at MIN (say, -2), jump to 0 (off)
    }
  } else {
    sound = fx_decrease;
    if (--SINK_ALARM < SINK_ALARM_MAX) {
      sound = fx_cancel;
      SINK_ALARM = 0;      // if we were at max, wrap back to 0
    } else if (SINK_ALARM > SINK_ALARM_MIN) {
      SINK_ALARM = SINK_ALARM_MIN; // if we were at 0, and dropped to -1, but still greater than the min (-2), jump to -2
    }
  }
	speaker_playSound(sound);
	// TODO: really needed? speaker_updateClimbToneParameters();	// call to adjust sinkRateSpread according to new SINK_ALARM value
}

void settings_adjustVarioAverage(int8_t dir) {
	uint16_t * sound = fx_neutral;
	
  if (dir > 0) {
    sound = fx_increase;    
    if (++VARIO_AVERAGE >= VARIO_AVERAGE_MAX) {
      VARIO_AVERAGE = VARIO_AVERAGE_MAX;  
      sound = fx_double;
    }
  } else {
    sound = fx_decrease;
    if (--VARIO_AVERAGE <= VARIO_AVERAGE_MIN) {
      VARIO_AVERAGE = VARIO_AVERAGE_MIN;
      sound = fx_double;
    }
  }
	speaker_playSound(sound);
}

// climb average goes between 0 and CLIMB_AVERAGE_MAX
void settings_adjustClimbAverage(int8_t dir) {
	uint16_t * sound = fx_neutral;
	
  if (dir > 0) {
    sound = fx_increase;    
    if (++CLIMB_AVERAGE >= CLIMB_AVERAGE_MAX) {
      CLIMB_AVERAGE = CLIMB_AVERAGE_MAX;  
      sound = fx_double;
    }
  } else {
    sound = fx_decrease;
    if (--CLIMB_AVERAGE <= 0) {
      CLIMB_AVERAGE = 0;
      sound = fx_double;
    }
  }
	speaker_playSound(sound);
}

void settings_adjustClimbStart(int8_t dir) {
	uint16_t * sound = fx_neutral;
	uint8_t inc_size = 5;

  if (dir > 0) {
    sound = fx_increase;    
    if ( (CLIMB_START += inc_size) >= CLIMB_START_MAX) {
      CLIMB_START = CLIMB_START_MAX;  
      sound = fx_double;
    }
  } else {
    sound = fx_decrease;
    if ( (CLIMB_START -= inc_size) <= 0) {
      CLIMB_START = 0;
      sound = fx_double;
    }
  }
	speaker_playSound(sound);
}



void settings_adjustLiftyAir(int8_t dir) {
	uint16_t * sound = fx_neutral;
	LIFTY_AIR += dir;

  if (dir > 0) {
    sound = fx_increase;
    if (LIFTY_AIR > 0) {      
      LIFTY_AIR = LIFTY_AIR_MAX;      // if we were at 0 and now are at positive 1, go back to max sink rate
    } else if (LIFTY_AIR == 0) {
      sound = fx_cancel;      
    }
  } else {
    sound = fx_decrease;
    if (LIFTY_AIR < LIFTY_AIR_MAX) {
      sound = fx_cancel;
      LIFTY_AIR = 0;    
    }
  }
	speaker_playSound(sound);	
}


void settings_adjustVolumeVario(int8_t dir) {

  uint16_t * sound = fx_neutral;
  
  if (dir > 0) {
    sound = fx_increase;
    VOLUME_VARIO++;
    if (VOLUME_VARIO > VOLUME_MAX) {
      VOLUME_VARIO = VOLUME_MAX;
      sound = fx_double;
    }
  } else {
    sound = fx_decrease;
    VOLUME_VARIO--;
    if (VOLUME_VARIO <= 0) {
      VOLUME_VARIO = 0;
      sound = fx_cancel;  // even if vario volume is set to 0, the system volume may still be turned on, so we have a sound for turning vario off
    }
  }
  speaker_playSound(sound); 
}
  

void settings_adjustVolumeSystem(int8_t dir) {

  uint16_t * sound = fx_neutral;
  
  if (dir > 0) {
    sound = fx_increase;
    VOLUME_SYSTEM++;
    if (VOLUME_SYSTEM > VOLUME_MAX) {
      VOLUME_SYSTEM = VOLUME_MAX;
      sound = fx_double;
    }
  } else {
    sound = fx_decrease;
    VOLUME_SYSTEM--;
    if (VOLUME_SYSTEM <= 0) {
      VOLUME_SYSTEM = 0;
      sound = fx_cancel;  // we have this line of code for completeness, but the speaker will be turned off for system sounds so you won't hear it
    }
  }
  speaker_playSound(sound); 
}

// swap unit settings and play a neutral sound
void settings_toggleUnits(bool * unitSetting) {
  *unitSetting = !*unitSetting;
  speaker_playSound(fx_neutral);
}

// flip on/off certain settings and play on/off sounds
void settings_toggleSwitch(bool * switchSetting) {
  *switchSetting = !*switchSetting;
  if (*switchSetting) speaker_playSound(fx_neutral);  // if we turned it on
  else speaker_playSound(fx_cancel);                  // if we turned it off
}







