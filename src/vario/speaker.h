#ifndef speaker_h
#define speaker_h

#include <Arduino.h>

/*  Old includes from AVR file
#include <avr/power.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "display.h"
#include "settings.h"
#include "LCD.h"
*/


//Pinout for Leaf V3.2.0
#define SPEAKER_PIN       7
#define SPEAKER_VOLA      8
#define SPEAKER_VOLB      9
#define PWM_CHANNEL       0   // ESP32 has many channels; we'll use the first

/*
// Pinout for Breakboard
#define SPEAKER_PIN		5   // output pin to play sound signal
#define SPEAKER_VOLA  4   // enable A pin for voltage amplification (loud)
#define SPEAKER_VOLB	6   // enable B pin for voltage amplification (louder) (enable both A & B for loudest)
#define PWM_CHANNEL   0   // ESP32 has many channels; we'll use the first
*/

//each "beep beep" cycle is a "measure", made up of play-length followed by rest-length, then repeat


// CLIMB TONE DEFINITIONS
#define CLIMB_AUDIO_THRESHOLD  10 	// don't play unless climb rate is over this value (cm/s)
#define CLIMB_MAX		          800		// above this cm/s climb rate, note doesn't get higher
#define CLIMB_NOTE_MIN		    500		// min tone pitch in Hz for >0 climb
#define CLIMB_NOTE_MAX 		   1600	  // max tone pitch in Hz for CLIMB_MAX (when vario peaks and starts holding a solid tone)
#define CLIMB_NOTE_MAXMAX    2200	  // max tone pitch in Hz when vario is truly pegged, even in solid-tone mode
#define CLIMB_PLAY_MAX		   1200 	// ms play per measure (at min climb)
#define CLIMB_PLAY_MIN		    200   // ms play per measure (at max climb)
#define CLIMB_REST_MAX       1000		// ms silence per measure (at min climb)
#define CLIMB_REST_MIN	      100		// ms silence per measure (at max climb)

// LiftyAir DEFINITIONS    (for air rising slower than your sinkrate, so net climbrate is negative, but not as bad as it would be in still air)
#define LIFTYAIR_TONE_MIN	    180		// min pitch tone for lift air @ -(setting)m/s
#define LIFTYAIR_TONE_MAX	    150		// max pitch tone for lifty air @ .1m/s climb
#define LIFTYAIR_PLAY	  	      1		//
#define LIFTYAIR_GAP		        1
#define LIFTYAIR_REST_MAX      20
#define LIFTYAIR_REST_MIN      10

// SINK TONE DEFINITIONS
#define SINK_ALARM           -050   // cm/s sink rate that triggers sink alarm audio
#define SINK_MAX		         -800		// at this sink rate, tone doesn't get lower
#define SINK_NOTE_MIN		      300   // highest tone pitch for sink > SINK_ALARM
#define SINK_NOTE_MAX		      110   // lowest tone pitch for sink @ SINK_MAX (when vario bottoms out and starts holding a solid tone)
#define SINK_NOTE_MAXMAX       70   // bottom tone pitch for sink (when vario is truly pegged, even in solid tone mode)

#define SINK_PLAY_MIN		     1200   // ms play per measure (at min sink)
#define SINK_PLAY_MAX		     2000 	// ms play per measure (at max sink)
#define SINK_REST_MIN	       1000		// silence samples (at min sink)
#define SINK_REST_MAX	       1000		// silence samples (at max sink)


void speaker_init(void);

void speaker_enableTimer(void);
void speaker_disableTimer(void);

void speaker_setVolume(unsigned char volume);

void speaker_updateVarioNote(int16_t verticalRate);
void speaker_updateClimbToneParameters(void);

void speaker_playSound(unsigned char sound);

void speaker_playsound_up(void);
void speaker_playsound_down(void);

void onSpeakerTimer(void);

void speaker_TEST(void);
void speaker_playPiano(void);

enum sound_tones {
  fx_silence,
	fx_increase,
	fx_decrease,
	fx_neutral,
  fx_neutralLong,
  fx_double,
  fx_enter,
  fx_exit,
  fx_confirm,
  fx_cancel,
  fx_on,
  fx_off,
  fx_buttonpress,
  fx_buttonhold,
  fx_goingup,
  fx_goingdown,
  fx_octavesup,
  fx_octavesdown
};

#define NOTE_B0 31      // will crash
#define NOTE_C1 33      // will crash
#define NOTE_CS1 35     // will crash
#define NOTE_D1 37      // will crash
#define NOTE_DS1 39     // will crash
#define NOTE_E1 41      // Lowest note we can play without crashing for some reason
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

#endif /* SPEAKER_H_ */