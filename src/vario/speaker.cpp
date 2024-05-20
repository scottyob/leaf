#include "speaker.h"

hw_timer_t *speaker_timer = NULL;

//Tone definitions
#define NOTE_END 1
#define NOTE_NONE 0

#define FX_NOTE_LENGTH  250

//Sound FX Tones
uint16_t st_silence[] = {NOTE_END};

uint16_t st_increase[] = {NOTE_C4, NOTE_G4, NOTE_END};
uint16_t st_decrease[] = {NOTE_C4, NOTE_F3, NOTE_END};   //110 140, END_OF_TONE};
uint16_t st_neutral[] = {NOTE_C4, NOTE_C4, NOTE_END};    //110, 110, END_OF_TONE};
uint16_t st_neutralLong[] = {NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_END}; //  {110, 110, 110, 110, 110, 110, 110, 110,10, 110, 1110, 110, 110, 110, 110, 110, 110, 110, 110, 110, END_OF_TONE};
uint16_t st_double[] = {NOTE_C4, NOTE_NONE, NOTE_C4, NOTE_END};  //110, 0, 110, END_OF_TONE};
uint16_t st_enter[] = {NOTE_A4, NOTE_C4, NOTE_E4, NOTE_END};     //150, 120, 90, END_OF_TONE};
uint16_t st_exit[] = {NOTE_C5, NOTE_A5, NOTE_F4, NOTE_C4, NOTE_END};       //65, 90, 120, 150, END_OF_TONE};


uint16_t st_confirm[] = {200, 200, 140, 140, 170, 170, 110, 110, NOTE_END};
uint16_t st_cancel[] = {150, 200, 250, NOTE_END};
uint16_t st_on[] = {250, 200, 150, 100, 50, NOTE_END};
uint16_t st_off[] = {50, 100, 150, 200, 250, NOTE_END};
uint16_t st_buttonpress[] = {180, 150, 120, NOTE_END};
uint16_t st_buttonhold[] = {150, 200, 250, NOTE_END};
uint16_t st_goingdown[] = {30, 31, 32, 33, 34, 35, 36, 38, 41, 46, NOTE_END};
uint16_t st_goingup[] = {55, 54, 53, 52, 51, 50, 49, 47, 44, 39, NOTE_END};
uint16_t st_octavesdown[] = {30, 30, 40, 40, 45, 45, 65, 65, 90, 90, NOTE_END};
uint16_t st_octavesup[] = {90, 90, 65, 65, 45, 45, 40, 40, 30, 30, NOTE_END};



uint16_t* sound_fx_tones[] = {
  st_silence,
  st_increase,
  st_decrease,
  st_neutral,
  st_neutralLong,
  st_double,
  st_enter,
  st_exit,
  st_confirm,
  st_cancel,
  st_on,
  st_off,
  st_buttonpress,
  st_buttonhold,
  st_goingup,
  st_goingdown,
  st_octavesup,
  st_octavesdown,
};

// volatile pointer to the sound sample to play
uint16_t* volatile snd_index;



volatile uint16_t sound_varioNote = 0;			    // note to play for vario beeps
volatile uint16_t sound_varioNoteLast = 0;		// last note played for vario beeps
volatile uint16_t sound_varioPlayLength = CLIMB_PLAY_MAX;		// 
volatile uint16_t sound_varioRestLength = CLIMB_REST_MAX;		// 

volatile uint16_t sound_fxNoteLast = 0;		// last note played for vario beeps

volatile bool sound_varioResting = 0;		              // are we resting (silence) between beeps?
volatile bool sound_currentlyPlaying = 0;         	// are we playing a sound?
volatile bool sound_fx = 0;                      // track if we have any sound effects to play (button presses, etc)

uint16_t random_note[] = {0, NOTE_END};

void speaker_init(void)
{
  //configure pinouts and PWM channel
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(SPEAKER_VOLA, OUTPUT);
  pinMode(SPEAKER_VOLB, OUTPUT);

  ledcSetup(PWM_CHANNEL, 1000, 8);
  ledcAttachPin(SPEAKER_PIN, PWM_CHANNEL);

	//set Volume to proper default setting
	//speaker_setVolume(VOLUME);

  //setup speaker timer interrupt to track each "beat" of sound
	speaker_timer = timerBegin(1, 320000, true);     // prescaler of 80,000 so timer ticks at 1KHz = 1ms per tick
  timerAttachInterrupt(speaker_timer, &onSpeakerTimer, true);
  timerAlarmWrite(speaker_timer, 1000, true);      
  //timerAlarmEnable(speaker_timer);

  speaker_enableTimer();
  Serial.println("enable timer - calling enable() in init");

	snd_index = st_silence;

	//speaker_updateClimbToneParameters();
 
  speaker_setVolume(1);  

}

void speaker_enableTimer(void) {
  timerAlarmEnable(speaker_timer);
  Serial.println("enable timer - enable timer function");
}

void speaker_disableTimer(void) {
  timerAlarmDisable(speaker_timer);	
  Serial.println("disable timer - disable timer function");
}

void speaker_setVolume(unsigned char volume) {
	switch (volume) {
		case 0:     // No Volume -- disable piezo speaker driver
      digitalWrite(SPEAKER_VOLA, 0);
      digitalWrite(SPEAKER_VOLB, 0);
      break;
		case 1:     // Low Volume -- enable piezo speaker driver 1x (3.3V)
			digitalWrite(SPEAKER_VOLA, 1);
      digitalWrite(SPEAKER_VOLB, 0);      
			break;
		case 2:     // Med Volume -- enable piezo speaker driver 2x (6.6V)
			digitalWrite(SPEAKER_VOLA, 0);
      digitalWrite(SPEAKER_VOLB, 1);      
			break;
		case 3:     // High Volume -- enable piezo spaker driver 3x (9.9V)
			digitalWrite(SPEAKER_VOLA, 1);
      digitalWrite(SPEAKER_VOLB, 1);
			break;
	}
  Serial.print("speaker_volume: ");
  Serial.println(volume);
}


void speaker_playSound(unsigned char sound)
{	
	snd_index = sound_fx_tones[sound];
  Serial.print("setting snd_index to: ");
  Serial.println(*snd_index);
  sound_fx = 1;
}

void speaker_playNote(uint16_t note) {
  random_note[0] = note;
  snd_index = random_note;
  sound_fx = 1;
}

unsigned char climbToneSpread;
unsigned int climbRateSpread;
unsigned int climbPlaySpread;
unsigned int climbSilenceSpread;

unsigned char sinkToneSpread;
unsigned int sinkRateSpread;
unsigned int sinkPlaySpread;
unsigned int sinkSilenceSpread;

unsigned char liftyToneSpread;
unsigned int liftyRateSpread;
unsigned int liftySilenceSpread;
unsigned char liftyAirGap = 1;		// track if we're playing the gap between double-beeps or the longer silence
unsigned char liftyAirNow = 0;		// track if we're playing lifty air tone

void speaker_updateClimbToneParameters(void)
{
  /*
	climbToneSpread = CLIMB_TONE_MIN - CLIMB_TONE_MAX;
	climbRateSpread = CLIMB_MAX - CLIMB_AUDIO_THRESHOLD;
	climbPlaySpread = CLIMB_PLAY_MAX - CLIMB_PLAY_MIN;
	climbSilenceSpread = CLIMB_SILENCE_MAX - CLIMB_SILENCE_MIN;

	sinkToneSpread = SINK_TONE_MIN - SINK_TONE_MAX;
	sinkRateSpread = SINK_MAX - ((unsigned int)SINK_ALARM * 100);
	sinkPlaySpread = SINK_PLAY_MAX - SINK_PLAY_MIN;
	sinkSilenceSpread = SINK_SILENCE_MAX - SINK_SILENCE_MIN;

	liftyToneSpread = LIFTYAIR_TONE_MIN - LIFTYAIR_TONE_MAX;
	liftyRateSpread = CLIMB_AUDIO_THRESHOLD + LIFTY_AIR;
	liftySilenceSpread = LIFTYAIR_SILENCE_MAX - LIFTYAIR_SILENCE_MIN;
  */
}

void speaker_updateVarioNote(int16_t verticalRate)
{
  if(verticalRate > CLIMB_AUDIO_THRESHOLD) {
    sound_varioNote = verticalRate * (CLIMB_NOTE_MAX - CLIMB_NOTE_MIN) / CLIMB_MAX + CLIMB_NOTE_MIN;
    sound_varioPlayLength = CLIMB_PLAY_MAX - (verticalRate * (CLIMB_PLAY_MAX - CLIMB_PLAY_MIN) / CLIMB_MAX);
    sound_varioRestLength = CLIMB_REST_MAX - (verticalRate * (CLIMB_REST_MAX - CLIMB_REST_MIN) / CLIMB_MAX);
  } else if (verticalRate < SINK_ALARM) {
    //do similar stuff for sink beeps
  }
  Serial.print("Note: ");
  Serial.print(sound_varioNote);
  Serial.print(" Play: ");
  Serial.print(sound_varioPlayLength);
  Serial.print(" Rest: ");
  Serial.println(sound_varioRestLength);


  /*
	unsigned char percentToMax = 0;

	sndCLIMB_tone = 0;	// assume no sound to play
	liftyAirNow = 0;	// assume no lifty air

	if (VOLUME != 0) {
		// if climbing
		if (VARIO_RATEfiltered >= CLIMB_AUDIO_THRESHOLD) {
			// calculate % between CLIMB_THRESHOLD (min climb) and CLIMB_MAX
			percentToMax = (VARIO_RATEfiltered - CLIMB_AUDIO_THRESHOLD)*100 / climbRateSpread;  //TODO: probably take out subtraction of the threshold, it's only 10cm
			if (percentToMax > 100) percentToMax = 100;

			// calculate sndCLIMB_tone, sndCLIMB_playCountMax, and sndCLIMB_silenceCountMax
			sndCLIMB_tone = CLIMB_TONE_MIN - (percentToMax * climbToneSpread / 100);
			sndCLIMB_playCountMax = CLIMB_PLAY_MAX - (percentToMax * climbPlaySpread / 100);
			sndCLIMB_silenceCountMax = CLIMB_SILENCE_MAX - (percentToMax * climbSilenceSpread / 100);

		// if Sink Alarm is ON and sinking > ALARM
		} else if (SINK_ALARM != 0 && VARIO_RATEfiltered <= -100*((long)SINK_ALARM)) {
			// calculate % between SINK_ALARM and SINK_MAX
			percentToMax = (-VARIO_RATEfiltered - ((unsigned int)SINK_ALARM*100)) * 100 / sinkRateSpread;
			if (percentToMax > 100) percentToMax = 100;

			// calculate sndCLIMB_tone, sndCLIMB_playCountMax, and sndCLIMB_silenceCountMax
			sndCLIMB_tone = SINK_TONE_MAX + (percentToMax * sinkToneSpread / 100);
			sndCLIMB_playCountMax = SINK_PLAY_MIN + (percentToMax * sinkPlaySpread / 100);
			sndCLIMB_silenceCountMax = SINK_SILENCE_MAX - (percentToMax * sinkSilenceSpread / 100);

		// if LiftyAir is ON and we're in that sink/climb range
		} else if (LIFTY_AIR != 0 && VARIO_RATEfiltered >= -LIFTY_AIR) {
			percentToMax = (-VARIO_RATEfiltered + CLIMB_AUDIO_THRESHOLD)*100  / liftyRateSpread;
			if (percentToMax > 100) percentToMax = 100;

			sndCLIMB_tone = LIFTYAIR_TONE_MAX + (percentToMax * liftyToneSpread / 100);
			sndCLIMB_playCountMax = LIFTYAIR_PLAY;
			sndCLIMB_silenceCountMax = LIFTYAIR_SILENCE_MIN + (percentToMax * liftySilenceSpread / 100);
			liftyAirNow = 1;
		}
	} 
  */
}


// timerWrite(timer, 0)

// Sound driver
void IRAM_ATTR onSpeakerTimer() {
  //first disable the alarm that got us here, until we're ready for the next one
  timerAlarmDisable(speaker_timer);
  //Serial.println("disable timer - top of interrupt");

  //prioritize sound effects from buttons etc before we get to vario beeps
  if (sound_fx) {								
    Serial.println("entering sound_fx in interrupt");    
		if (*snd_index != NOTE_END) {
			if (*snd_index != sound_fxNoteLast) ledcWriteTone(PWM_CHANNEL, *snd_index);   // only change pwm if it's a different note, otherwise we get a little audio blip between the same notes
      sound_fxNoteLast = *snd_index;
      timerAlarmWrite(speaker_timer, FX_NOTE_LENGTH, true); // start timer for play period
      timerAlarmEnable(speaker_timer);
      Serial.println("enable timer - in bewteen FX tones");			

      Serial.print("Index Tone: ");
      Serial.print(*snd_index);
      Serial.print(" Length: ");
      Serial.print(FX_NOTE_LENGTH);
      Serial.println(" finished sound_fx in interrupt");

      snd_index++;
		} else {									// Else, we're at END_OF_TONE
      Serial.println("disable timer - end of FX");
      ledcWriteTone(PWM_CHANNEL, 0);    
			timerAlarmDisable(speaker_timer);
			sound_fx = 0;
      sound_fxNoteLast = 0;
		}    
  } else if (sound_varioNote > 0) {
    // Handle the beeps and rests of a vario sound "measure"
    if (sound_varioResting) {
      ledcWriteTone(PWM_CHANNEL, 0);                // "play" silence since we're resting between beeps
      sound_varioResting = false;                      // next time through we want to play sound
      timerAlarmWrite(speaker_timer, sound_varioRestLength, true); // start timer for the rest period
      timerAlarmEnable(speaker_timer);              // ...and go!
      Serial.println("enable timer - vario Rest");			
    } else {
      ledcWriteTone(PWM_CHANNEL, sound_varioNote);  // play the note
      sound_varioResting = true;                       // next time through we want to rest (play silence)
      timerAlarmWrite(speaker_timer, sound_varioPlayLength, true); // start timer for play period
      timerAlarmEnable(speaker_timer);
      Serial.println("enable timer - vario beep");			
    }
  }
/*

  
	




	} else if (sound_climbNote > 0) {					// Play climb sound if desired, if there were no FX to play
		if (!sndCLIMB_silence) {					// 	if not playing silence..
			
      timerAlarmWrite(speaker_timer, 1000, true);

			if (!sndCurrentlyPlaying) {				// if we're not playing a beep currently, then go ahead and allow a frequency/pitch change
				ledcWriteTone(PWM_CHANNEL, sound_climbNote);				//  this is the note we play
				sndCurrentlyPlaying = 1;			//	and note that we're playing something
			}
			
			sndCLIMB_playCount++;					//	  log this play of the note

			if (sndCLIMB_playCount >= sndCLIMB_playCountMax) {	// if we've played the note enough times
				sndCLIMB_playCount = 0;							//   reset play count
				sndCurrentlyPlaying = 0;
				if (sndCLIMB_silenceCountMax >= 1)				//   and if we need silence
					sndCLIMB_silence = 1;						//    get ready to play silence
			}

		} else {
			speaker_disableTimer();					// play silence			
			sndCLIMB_silenceCount++;				// log this play of silence
			if (sndCLIMB_silenceCount >= sndCLIMB_silenceCountMax) { 	// if we've played silence long enough
				sndCLIMB_silence = 0;									//  time for noise
				sndCLIMB_silenceCount = 0;								//  reset silence count
				if (liftyAirNow) liftyAirGap = 1;
			} else if (liftyAirNow && liftyAirGap && sndCLIMB_silenceCount >= LIFTYAIR_GAP) {
				sndCLIMB_silence = 0;									//  time for noise
				sndCLIMB_silenceCount = 0;								//  reset silence count
				liftyAirGap = 0;
			}
		}

	} else {										// No sound to play
		speaker_disableTimer();						//   turn off sound
	}
  */
}

void speaker_TEST(void) {
  //Serial.println('0');
  
  //Explore sounds
  speaker_playPiano();  
  

  //Test sound fx
  /*
  for (int i=0; i<20; i++) {
    sound_fx = 1;
    Serial.print("now called playSound()");
    Serial.println(i);
    speaker_playSound(i);
    delay(4000);
  }
  */


  //Test Vario Beeps
  /*
  for (int16_t i=11; i<900; i+=50) {
    speaker_updateVarioNote(i);  
    delay(3000);
  }
  */
}

void speaker_playPiano(void) {
if (Serial.available() > 0) {
    char letter = char(Serial.read());
    while (Serial.available() > 0) {
      Serial.read();
    }

    Serial.println(letter);
    int fx = 0;
    switch (letter) {
      case 'z': fx = NOTE_A2; break;
      case 'x': fx = NOTE_B2; break;
      case 'c': fx = NOTE_C3; break;
      case 'v': fx = NOTE_D3; break;
      case 'b': fx = NOTE_E3; break;
      case 'n': fx = NOTE_F3; break;
      case 'm': fx = NOTE_G3; break;
      case ',': fx = NOTE_A3; break;
      case '.': fx = NOTE_B3; break;      
      case 'a': fx = NOTE_A3; break;
      case 's': fx = NOTE_B3; break;
      case 'd': fx = NOTE_C4; break;
      case 'f': fx = NOTE_D4; break;
      case 'g': fx = NOTE_E4; break;
      case 'h': fx = NOTE_F4; break;
      case 'j': fx = NOTE_G4; break;
      case 'k': fx = NOTE_A4; break;
      case 'l': fx = NOTE_B4; break;
      case ';': fx = NOTE_C5; break;            
      case 'q': fx = NOTE_A4; break;
      case 'w': fx = NOTE_B4; break;
      case 'e': fx = NOTE_C5; break;
      case 'r': fx = NOTE_D5; break;
      case 't': fx = NOTE_E5; break;
      case 'y': fx = NOTE_F5; break;
      case 'u': fx = NOTE_G5; break;
      case 'i': fx = NOTE_A5; break;
      case 'o': fx = NOTE_B5; break;
      case 'p': fx = NOTE_C6; break;      
      case '[': fx = NOTE_D6; break;      
      case ']': fx = NOTE_E6; break;      
      case '0': speaker_setVolume(0); break;      
      case '1': speaker_setVolume(1); break;      
      case '2': speaker_setVolume(2); break;      
      case '3': speaker_setVolume(3); break;            
    }
    speaker_playNote(fx);
  }
}
