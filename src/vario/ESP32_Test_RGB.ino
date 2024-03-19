#include <Arduino.h>
#include <NMEAGPS.h>
#include "pressure.h"
#include "Leaf_SPI.h"

#include <HardwareSerial.h>
HardwareSerial gpsSerial(1); // define a Serial for UART1
const int gpsSerialRX = 44;
const int gpsSerialTX = 43;

// RGB LED STUFF
#define LED_PIN 48
#define NUM_LEDS 1
#define BRIGHTNESS 10
#define LED_TYPE WS2812
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
CRGB leds[NUM_LEDS];

//should move to SPI file later
#define LCD_RS    46          // RS pin for data or instruction


// Main Timer Setup and interrupt event
hw_timer_t *Timer0_Cfg = NULL;
char pressureUpdateIndex = 0;   // keep track of which pressure udpate we're doing (it takes 3+ timed calls to the pressure sensore to get a full cycle data)
char timerIndex_10ms = 0;       // keep track of how many timer-alarm-events we've processed (each count should be 10ms)
char timerIndex_500ms = 0;      // keep track of how many half-second triggers have passed

void IRAM_ATTR Timer0_ISR() {
  //do stuff every alarm cycle (default 10ms)
    //Pressure Update 
    PressureUpdate(pressureUpdateIndex++);
    if(pressureUpdateIndex >=3) pressureUpdateIndex = 0;

    //Poll Buttons
    //pollButtons();

    //Index the main timer
    timerIndex_10ms++;    
    if(timerIndex_10ms >= 5) {
      timerIndex_10ms = 0

      //do stuff every 5 alarm cycles (every 0.5 seconds)
      //displayUpdate();
    }


}


void setup()
{

//Start Main System Timer for Interrupt Events
  Timer0_Cfg = timerBegin(0, 80, true);                 // Prescaler of 80, so 80Mhz drops to 1Mhz
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);  // Attach interrupt to handle alarm events
  timerAlarmWrite(Timer0_Cfg, 100000, true);            // Set alarm to go every 100,000 ticks (every 0.1 seconds)
  timerAlarmEnable(Timer0_Cfg);                         // Enable alarm & timer

// should move to SPI later
pinMode(LCD_RS, OUTPUT);

// Start USB Serial Debugging Port
Serial.begin(115200);
delay(1000);
Serial.println("Starting");

// Start GPS UART port
gpsSerial.begin(4800, SERIAL_8N1, gpsSerialRX, gpsSerialTX);

setup_Leaf_SPI();

// LED setup for dev kit board addressible RGB led
FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);
FastLED.setBrightness(BRIGHTNESS );

//Initialize devices
pressure_init();
LCD_init();
GPS_init();


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {

pressure_update1();
delay(15);
pressure_update2();
delay(15);
pressure_update3();
pressure_update4();


Serial.print("T: ");
Serial.print(TEMP);
Serial.print("  T_filt: ");
Serial.print(TEMPfiltered);
Serial.print("  P: ");
Serial.print(PRESSURE);
Serial.print("  P_filt: ");
Serial.print(PRESSUREfiltered);
Serial.print("  Alt: ");
Serial.print(P_ALT);
Serial.print("  Alt_filt: ");
Serial.print(P_ALTfiltered);
Serial.println(" ");


delay(450);


// Rebroadcast GPS serial data to debugger port
/*
while (gpsSerial.available() > 0) {
//Serial.print(char(gpsSerial.read()));
//Serial.print("_");
}
*/

//Serial.println("waiting");
//delay(500);

// Blink RGB light
/*
Serial.println("Red");
leds[0] = CRGB::Red;
FastLED.show(); // << failes here; have also tried FastLED.delay(100); as well as following FastLED.show(); with yield();
delay(1000);

Serial.println("Green");
leds[0] = CRGB::Green;
FastLED.show();
delay(1000);

Serial.println("Blue");
leds[0] = CRGB::Blue;
FastLED.show();
delay(1000);
*/

}

// Initialize the LCD
void LCD_init(void)
{
	delay(20);
	LCD_inst(0b00111001); //8 bit data, 2 line (ram), instr. table 1  //was 0x39 / 0b00111001
	delay(20);
	LCD_inst(0x15); // 0x14 for 2-line, 0x15 for 3-line
	delay(20);
	LCD_inst(0b01010101); // icon/boost/contrast 0101 Icon Boost C5 C4  // was 0x51 / 0b01010001 icon off, booster off, c5/c4 off
	delay(20);
	LCD_inst(0b01101101); // voltage follower on / 0110 Fon Rab2 Rab1 Rab0 / (+ ratio/gain) // *1101 for 2-line, *1110 for 3-line
	delay(20);
	//LCD_contrast(CONTRAST); // contrast set 0111 C3 C2 C1 C0   // ds suggests: 0b01110010
	//_delay_ms(10); // was 100
  //	LCD_inst(0b00111001); 				//8 bit data, 2 line (ram), instr. table 1  //was 0x39 / 0b00111001
	//delay(20);
	LCD_inst(0b01111011);   				// contrast set 0111 C3 C2 C1 C0
	delay(100);
	LCD_inst(0b00111000); //8 bit data, 2 line (ram), instr. table 0  //was 0x38 / 0b00111000
	delay(100); // was 500
	LCD_inst(0x0F); // disp. on, cursor on, blink
	delay(20);
	LCD_inst(0x01); // CLEAR DISPLAY cursor home
	delay(20);
	LCD_inst(0x06); // cursor direction + shift (0)
	delay(20);
}

// Send data to LCD
void LCD_data(unsigned char d)
{
	digitalWrite(LCD_RS, HIGH);	// Pull RS high to indicate data
	//LCD_write(d);
  LCD_spiCommand(d);
}

// Send instruction to LCD
void LCD_inst(unsigned char i)
{
	digitalWrite(LCD_RS, LOW);	// Pull RS low to indicate command instruction
	//LCD_write(i);
  LCD_spiCommand(i);
}


void GPS_sendCommand(char* string) {
  unsigned char c;
	c = *string++;

	while ( c != '\0' )	{
		gpsSerial.print(c);
    Serial.print((char)c);
		c = *string++;
	}

	//carriage return and line feed required to end NMEA sentence
  gpsSerial.print('\r');
  Serial.print('\r');
	gpsSerial.print('\n');
  Serial.print('\n');
}

void GPS_init()
{

// turn off unneeded NMEA messages
Serial.println("Setting GPS messages");
	delay(1000);
GPS_sendCommand("$PSRF103,00,00,01,01*25");	//turn on GGA at 1 sec (1st message by default)
GPS_sendCommand("$PSRF103,01,00,00,01*25");	//turn off GLL
GPS_sendCommand("$PSRF103,02,00,00,01*26");	//turn off GSA (2nd message by default)
GPS_sendCommand("$PSRF103,03,00,00,00*26");	//turn off GSV at 4 sec (3rd message by default, up to 3 sentences)  //was 00,01*23"
GPS_sendCommand("$PSRF103,04,00,01,01*21");	//turn on RMC at 1 sec (4th message by default)
GPS_sendCommand("$PSRF103,05,00,00,01*21");	//turn off VTG

Serial.println("Completed");

  /*
	// set ON_OFF pin as an output, default low.  And nWAKEUP pin as an input
	GPS_DDR |= GPS_ON_OFF;
	GPS_PORT &= ~GPS_ON_OFF;
	GPS_DDR &= ~GPS_nWAKEUP;

	// set up GPS reset pin, and default to high (to reset, pull pin low)
	GPS_RESET_DDR |= GPS_nRST;
	GPS_RESET_PORT |= GPS_nRST;

	// Get GPS turned on to configure (if user setting has GPS set to OFF, we'll turn it off once we're done here)
	gps_enable();

	// set UART baudrate to the GPS default baudrate
	uart_setBaud(BAUD_DEFAULT_PRESCALE);

	// configure GPS to use max baudrate, NMEA mode
	_delay_ms(1000);
	gps_txStringChecksum("$PSRF100,1,38400,8,1,0*00");
	_delay_ms(50);

	// now configure uart to use max baudrate too
	uart_setBaud(BAUD_MAX_PRESCALE);

	// turn off unneeded NMEA messages
	gps_txStringChecksum("$PSRF103,05,00,00,01*00");	//turn off VTG
	gps_txStringChecksum("$PSRF103,02,00,00,01*00");	//turn off GSA
	gps_txStringChecksum("$PSRF103,01,00,00,01*00");	//turn off GLL

	//turn on desired messages
	gps_txStringChecksum("$PSRF103,00,00,01,01*00");	//turn on GGA at 1 sec
	gps_txStringChecksum("$PSRF103,04,00,01,01*00");	//turn on RMC at 1 sec
	gps_txStringChecksum("$PSRF103,03,00,04,01*00");	//turn on GSV at 4 sec

    // Initialize the GPS sentence string position to 0
    gpsstrpos = 0;

    // Initialize GPSdata track_string struct
    gps_init_ts(&GPSdata);

    // Initialize GPSsat satellite_string struct and showSatSearch[8] string
    gps_init_ss(&GPSsat);

    // Set status default
    gps_status = gps_searching;


    // Now that GPS is initialized, put it to sleep if user has it set to OFF.
    if (GPS_USER_SETTING == 0)
    	gps_disable();
*/
}