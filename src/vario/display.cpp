/*
 * display.cpp
 *
 *
 */
#include <Arduino.h>
#include "display.h"
#include "display_tests.h"
#include "Leaf_SPI.h"

#define GLCD_RS LCD_RS
#define GLCD_RESET LCD_RESET


// Display Testing Temp Vars
float wind_angle = 1.57;
char seconds = 0;
char minutes = 0;
char hours = 0;
uint16_t heading = 0;
char string_heading[] = " WNW ";
char string_satNum[] = "00";
char string_gpsLat[] = "0000000000";
char string_gpsLng[] = "0000000000";



U8G2_ST7539_192X64_F_4W_HW_SPI u8g2(U8G2_R3, SPI_SS_LCD, LCD_RS, LCD_RESET);



void display_init(void) {
  digitalWrite(SPI_SS_LCD, HIGH);
  u8g2.setBusClock(20000000);
  u8g2.begin();
  u8g2.setContrast(80);

pinMode(LCD_BACKLIGHT, OUTPUT);

}




// Initialize the GRAPHIC LCD
void GLCD_init(void)
{
  pinMode(GLCD_RS, OUTPUT);
	digitalWrite(GLCD_RESET, LOW);
  delay(500);
  digitalWrite(GLCD_RESET, HIGH);
  delay(500);
	GLCD_inst(0b11100010); //Reset
  delay(20);
  GLCD_inst(0b10101111); //Enable
  delay(20);
  //GLCD_inst(0b10000001); //set Contrast
  //GLCD_inst(0b00001111); //contrast value
  delay(20);
  GLCD_inst(0b00000000);  //Column address LSB ->0
  delay(20);
  GLCD_inst(0b00010000);  //Column address MSB ->0
  delay(20);
  GLCD_inst(0b10110000);  //Page address ->0
  delay(20);
  GLCD_inst(0b10100110);  //Set inverse display->NO
  delay(20);

  for (int i=0; i<1536; i++){
    GLCD_data(0b00000000);    //clear LCD
  }
}



void display_battery_icon(uint16_t x, uint16_t y, uint8_t pct, bool charging) {

  uint8_t w =  7;
  uint8_t h = 12;  

  u8g2.setDrawColor(1);      
  u8g2.drawFrame(x,y+1,w,h-1);               // main battery box
  u8g2.drawLine(x+w/3, y, x+w-w/3-1, y);     // little +nib tip
  uint16_t fill = (h-2)*pct/100;
  u8g2.drawBox(x+1, y+h-fill-1, w-2, fill);  // fill up to capacity

  if(charging) {                             // draw lightning bolt if charging
    for (int i = 0; i<7; i++) {
      if ( (y+3+i) >= (y+h-fill-1) ) u8g2.setDrawColor(0); //invert lightning bolt where battery level is filled
      u8g2.drawPixel(x+w/2,y+3+i);
      if (i==2) u8g2.drawPixel(x+w/2-1,y+3+i);
      if (i==3) {u8g2.drawPixel(x+w/2-1,y+3+i); u8g2.drawPixel(x+w/2+1,y+3+i);}
      if (i==4) u8g2.drawPixel(x+w/2+1,y+3+i);
    }
    u8g2.setDrawColor(1); //end invert    
  }
}

void display_test_bat_icon(void) {
  uint8_t x = 1;
  uint8_t y = 1;
  uint8_t pct = 0;
  uint8_t w =  7;
  uint8_t h = 12; 
  bool charging = 1;


  u8g2.firstPage();
  do {
    for (int i=0; i<51; i++) {
      u8g2.setDrawColor(1);      
      u8g2.drawFrame(x,y+1,w,h-1);           // main battery box
      u8g2.drawLine(x+w/3, y, x+w-w/3-1, y); // little +nib tip
      uint16_t fill = (h-2)*pct/100;
      u8g2.drawBox(x+1, y+h-fill-1, w-2, fill);  // fill up to capacity

      // draw lightning bolt if charging
      if(charging) {
        for (int i = 0; i<7; i++) {
          if ( (y+3+i) >= (y+h-fill-1) ) u8g2.setDrawColor(0); //invert lightning bolt where battery level is filled
          u8g2.drawPixel(x+w/2,y+3+i);
          if (i==2) u8g2.drawPixel(x+w/2-1,y+3+i);
          if (i==3) {u8g2.drawPixel(x+w/2-1,y+3+i); u8g2.drawPixel(x+w/2+1,y+3+i);}
          if (i==4) u8g2.drawPixel(x+w/2+1,y+3+i);
        }
        u8g2.setDrawColor(1); //end invert
        
      }

      x += 10;
      pct += 2;
      if (x >= 63-w) {
        x = 1;
        y += h+2;
      }  
    }
  } while ( u8g2.nextPage() );
  delay(100);
}

// draw satellite constellation starting in upper left x, y and box size (width = height)
void display_satellites(uint16_t x, uint16_t y, uint16_t size) {
  Serial.println("entering display_satellites");
  u8g2.firstPage();
  do {
    // Draw the background
    u8g2.setDrawColor(0);
    u8g2.drawBox(x, y, size, size);   // clear the box drawing area
    u8g2.setDrawColor(1);
    u8g2.drawCircle(x+size/2, y+size/2, size/2);   // the horizon circle
    u8g2.drawCircle(x+size/2, y+size/2, size/4);   // the 45deg elevation circle

    // Draw the satellites
    for (int i = MAX_SATELLITES -1; i>=0; i--) { 
      if (sats[i].active) {
        /*
        Serial.print("act: ");
        Serial.print(sats[i].active);
        Serial.print(" el: ");
        Serial.print(sats[i].elevation);
        Serial.print(" az: ");
        Serial.print(sats[i].azimuth);
        Serial.print(" snr: ");
        Serial.println(sats[i].snr);
        */

        // Sat location (on circle display)
        uint16_t radius = (90 - sats[i].elevation) * size/2 / 90;
        int16_t sat_x = sin(sats[i].azimuth*PI/180)*radius;
        int16_t sat_y = - cos(sats[i].azimuth*PI/180)*radius;

        // Draw disc
        /*
        u8g2.drawDisc(size/2+sat_x, size/2+sat_y, size/16);
        u8g2.setDrawColor(0);
        u8g2.drawCircle(size/2+sat_x, size/2+sat_y, size/16+1);
        u8g2.setDrawColor(1);
        */

        // Draw box with numbers
        uint16_t x_pos = x + size/2 + sat_x;
        uint16_t y_pos = y + size/2 + sat_y;

        u8g2.setFont(u8g2_font_micro_tr);          // Font for satellite numbers
        if (sats[i].snr < 20) {                    
          u8g2.drawFrame(x_pos-5, y_pos-4, 11, 9); // white box with black border if SNR is low
          u8g2.setDrawColor(0);
          u8g2.drawBox(x_pos-4, y_pos-3, 9, 7); // erase the gap between frame and text
          u8g2.setDrawColor(1);
        } else {
          u8g2.drawBox(x_pos-4, y_pos-3, 9, 7);    // black box if SNR is high
          u8g2.setDrawColor(0);                    // .. with white font inside box
        }

        if (i<9) {
          u8g2.drawStr(x_pos-3, y_pos+3, "0");
          x_pos += 4;
        }
        u8g2.drawStr(x_pos-3, y_pos+3, itoa(i+1,string_satNum, 10));
        u8g2.setDrawColor(1);                
      }
     
    }
    
   
    //draw other GPS stuff just for testing purposes
    u8g2.drawStr(0, size + y + 10, "Lat: ");
    u8g2.drawStr(20, size + y + 10, itoa(gps.location.lat(), string_gpsLat, 10));

    u8g2.drawStr(0, size + y + 20, "Lon: ");
    u8g2.drawStr(20, size + y + 20, itoa(gps.location.lng(), string_gpsLng, 10));

    //u8g2.drawStr(0, size + y + 30, "Speed: ");
    //u8g2.drawStr(50, size + y + 30, gps.speed());

    u8g2.drawStr(0, size + y + 40, "Heading: ");
    u8g2.drawStr(50, size + y + 40, gps.cardinal(gps.course.deg()));
  } while ( u8g2.nextPage() );
  Serial.println("exiting display_satellites");
}


void display_test(void) {  
  delay(100);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_tinyunicode_tf);
    u8g2.drawStr(49, 18, "mph");

    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(44, 12, "103");       // speed

    
    delay(30);
    //heading++;
    heading = wind_angle*360/(2 * PI)+180;
    heading = heading % 360;
    
    
    if (heading == 360) heading = 0;
    if (heading < 12 || heading >= 349) strcpy(string_heading, "  N >"); 
    else if (heading < 34 ) strcpy(string_heading, " NNE>");
    else if (heading < 57 ) strcpy(string_heading, "  NE>");
    else if (heading < 79 ) strcpy(string_heading, " ENE>");
    else if (heading < 102) strcpy(string_heading, "  E >");
    else if (heading < 124) strcpy(string_heading, " ESE>");
    else if (heading < 147) strcpy(string_heading, "  SE>");
    else if (heading < 169) strcpy(string_heading, " SSE>");
    else if (heading < 192) strcpy(string_heading, "< S  "); 
    else if (heading < 214) strcpy(string_heading, "<SSW ");
    else if (heading < 237) strcpy(string_heading, "< SW ");
    else if (heading < 259) strcpy(string_heading, "<WSW ");
    else if (heading < 282) strcpy(string_heading, "< W  ");
    else if (heading < 304) strcpy(string_heading, "<WNW ");
    else if (heading < 327) strcpy(string_heading, "< NW ");
    else if (heading < 349) strcpy(string_heading, "<NNW ");
    else strcpy(string_heading, "<999>");



    u8g2.drawStr( 0, 12, string_heading);  // direction


    uint16_t nav_circle_center_x = 26;
    uint16_t nav_circle_center_y = 41;
    uint16_t nav_circle_radius   = nav_circle_center_x;   // set to x dimension for left-justified nav circle

    u8g2.drawCircle(nav_circle_center_x,nav_circle_center_y,nav_circle_radius);
    u8g2.drawCircle(nav_circle_center_x,nav_circle_center_y,nav_circle_radius-1);
    u8g2.drawCircle(nav_circle_center_x,nav_circle_center_y,nav_circle_radius*2/3);
    u8g2.drawCircle(nav_circle_center_x,nav_circle_center_y,nav_circle_radius*1/3);
  
    // Wind Triangle
    uint16_t wind_triangle_radius = 10;
    wind_angle += .01;
    if (wind_angle > 2 * PI) wind_angle = 0;

    uint16_t wind_triangle_center_x = nav_circle_center_x;
    uint16_t wind_triangle_center_y = nav_circle_center_y+nav_circle_radius/2;    
    int16_t wind_triangle_tip_len = wind_triangle_radius-1;  // pixels from center for point
    int16_t wind_triangle_tail_len = wind_triangle_radius-1;  // pixels from center for the tails
    float wind_triangle_tail_angle = 0.65;  // pixels tail width (half-width)

    u8g2.setDrawColor(0);
    u8g2.drawDisc(wind_triangle_center_x, wind_triangle_center_y, wind_triangle_radius);
    u8g2.setDrawColor(1);
    u8g2.drawCircle(wind_triangle_center_x, wind_triangle_center_y, wind_triangle_radius);        
    u8g2.drawTriangle(32, 96, 36, 106, 28, 106);

    uint16_t tip_xprime = wind_triangle_center_x + sin(wind_angle + PI)*wind_triangle_tip_len;
    uint16_t tip_yprime = wind_triangle_center_y - cos(wind_angle + PI)*wind_triangle_tip_len;
    uint16_t tail_1_xprime = wind_triangle_center_x + sin(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_1_yprime = wind_triangle_center_y - cos(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_2_xprime = wind_triangle_center_x + sin(wind_angle - wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_2_yprime = wind_triangle_center_y - cos(wind_angle - wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_mid_xprime = wind_triangle_center_x + sin(wind_angle)*wind_triangle_tail_len/2;
    uint16_t tail_mid_yprime = wind_triangle_center_y - cos(wind_angle)*wind_triangle_tail_len/2;
    
    u8g2.drawTriangle(tip_xprime, tip_yprime, tail_1_xprime, tail_1_yprime, tail_mid_xprime, tail_mid_yprime);
    u8g2.drawTriangle(tip_xprime, tip_yprime, tail_2_xprime, tail_2_yprime, tail_mid_xprime, tail_mid_yprime);
    u8g2.drawLine(tip_xprime, tip_yprime, tail_1_xprime, tail_1_yprime);
    u8g2.drawLine(tail_mid_xprime, tail_mid_yprime, tail_2_xprime, tail_2_yprime);


/*

    u8g2.drawTriangle(uint16_t(triangle_center_x + sin(wind_angle + PI)*wind_triangle_tip_len),                           // x' tip
                      uint16_t(triangle_center_y - cos(wind_angle + PI)*wind_triangle_tip_len),                           // y' tip
                      uint16_t(triangle_center_x + sin(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len),    // x' tail 1
                      uint16_t(triangle_center_y - cos(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len),    // y' tail 1
                      uint16_t(triangle_center_x + sin(wind_angle)*wind_triangle_tail_len/2),                             // x' middle tail
                      uint16_t(triangle_center_y - cos(wind_angle)*wind_triangle_tail_len/2)                              // y' middle tail
                      );
    u8g2.drawTriangle(uint16_t(triangle_center_x + sin(wind_angle + PI)*wind_triangle_tip_len),                           // x' tip
                      uint16_t(triangle_center_y - cos(wind_angle + PI)*wind_triangle_tip_len),                           // y' tip
                      uint16_t(triangle_center_x + sin(wind_angle)*wind_triangle_tail_len/2),                             // x' middle tail
                      uint16_t(triangle_center_y - cos(wind_angle)*wind_triangle_tail_len/2),                             // y' middle tail
                      uint16_t(triangle_center_x + sin(wind_angle - wind_triangle_tail_angle)*wind_triangle_tail_len),    // x' tail 2
                      uint16_t(triangle_center_y - cos(wind_angle - wind_triangle_tail_angle)*wind_triangle_tail_len)     // y' tail 2
                      );
    u8g2.drawLine(    uint16_t(triangle_center_x + sin(wind_angle + PI)*wind_triangle_tip_len),                           // x' tip
                      uint16_t(triangle_center_y - cos(wind_angle + PI)*wind_triangle_tip_len),                           // y' tip
                      uint16_t(triangle_center_x + sin(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len),    // x' tail 1
                      uint16_t(triangle_center_y - cos(wind_angle + wind_triangle_tail_angle)*wind_triangle_tail_len)    // y' tail 1                     
                      );
*/

/*
    u8g2.drawTriangle(  uint16_t(cos(angle)*wind_triangle_tip_len) + triangle_center_x,       // x' tip
                        uint16_t(sin(angle)*wind_triangle_tip_len) + triangle_center_y,      // y' tip
                        uint16_t(cos(angle)*wind_triangle_width + sin(angle)*wind_triangle_tail_len) + triangle_center_x,  // x' right tail
                        uint16_t(-cos(angle)*wind_triangle_tail_len + sin(angle)*wind_triangle_width) + triangle_center_y, // y' right tail
                        uint16_t(-cos(angle)*wind_triangle_width + sin(angle)*wind_triangle_tail_len) + triangle_center_x, // x' left tail
                        uint16_t(-cos(angle)*wind_triangle_tail_len - sin(angle)*wind_triangle_width) + triangle_center_y // y' right tail
                        );
  */  


                        


    // Waypoint Name
    u8g2.setFont(u8g2_font_12x6LED_tf);
    u8g2.drawStr(0, 80, "MarshallLZ");




    // Timer w/ Inverted Colors
    u8g2.drawRBox(0,146,51,16,3);
    u8g2.setDrawColor(0);
    u8g2.setFontMode(1);
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(2, 160, "12:34:00");
    u8g2.setFontMode(0);
    u8g2.setDrawColor(1);


    u8g2.setFont(u8g2_font_12x6LED_mn);
    u8g2.drawStr(0, 180, "12:34:00");
    
    //u8g2.drawStr(0,83,"12,345\"");      // altitude


    int v = 17;
    int width = 16;
    /*
    u8g2.drawFrame(64-width, 96, width, 48);
    u8g2.drawFrame(64-width, 144, width, 48);
    u8g2.drawBox(64-width, 144-v, width, v);
    */
  } while ( u8g2.nextPage() );  
}


void display_test_big(uint8_t test_page) {
  
//Serial.println("starting LCD stuff");
/* NOT SURE WHAT THIS IS
GLCD_inst(0b10100101);
delay(500);
GLCD_data(0b10100101);
delay(500);
GLCD_inst(0b10100100);
delay(500);
GLCD_data(0b10100100);
delay(500);
*/

/*


  //GO HOME GLCD
  GLCD_inst(0b00000000);  //Column address LSB ->0
  delay(20);
  GLCD_inst(0b00010000);  //Column address MSB ->0
  delay(20);
  GLCD_inst(0b10110000);  //Page address ->0
  delay(20);
*/


  if (test_page == 1) {
    for (int page=0; page<8; page++) {
      for (int d=0; d<192; d++) {
        GLCD_data(vario_main_1[d+page*192]);  
      }
    }    
  } else if (test_page == 2) {
    for (int page=0; page<8; page++) {
      for (int d=0; d<192; d++) {
        GLCD_data(vario_nav_1a[d+page*192]);  
      }
    }    
  } else if (test_page == 3) {
    for (int page=0; page<8; page++) {
      for (int d=0; d<192; d++) {
        GLCD_data(vario_nav_1b[d+page*192]);  
      }
    }   
  } else if (test_page == 4) {
    for (int page=0; page<8; page++) {
      for (int d=0; d<192; d++) {
        GLCD_data(vario_nav_2[d+page*192]);  
      }
    }   
  } else if (test_page == 5) {
    for (int page=0; page<8; page++) {
      for (int d=0; d<192; d++) {
        GLCD_data(offroad_1[d+page*192]);  
      }
    }   
  }


  //digitalWrite(LCD_BACKLIGHT, !digitalRead(LCD_BACKLIGHT));
  //delay(500);
//BIG LCD TEST 
}


void GLCD_inst(byte data) {
  digitalWrite(GLCD_RS, LOW);
  GLCD_spiCommand(data);
}

void GLCD_data(byte data) {
  digitalWrite(GLCD_RS, HIGH);
  GLCD_spiCommand(data);
}


char speed[] = "132";
char windSpeed[] = "28";
char turn = 1;
uint16_t windDir = 235;

char altitude[] = "23,857\"";
char altAbvLaunch[] = "4,169";
char glide[] = "10.4";
char distFlown[] = "34.5";
char glideToWypt[] = " 6.5";
char timeToWypt[] = "12:34";
char distToWypt[] = "42.7";
char waypoint[] = "Marshall-LZ";
char temp[] = "102";
char accel[] = "2.1g";
char altAbvLZ[] = "1,987";
char climbRate[] = "+1385";
char clockTime[] = "12:57pm";
char timer[] = "1:23:45";
float dirToWypt = -.25;


/*
void display_test_real() {
  u8g2.firstPage();
  do {
    

    // heading and turn
    u8g2.setFont(leaf_7x10);
    //if (turn == 0) string_heading[0] = '<';
    //if (turn == 1) string_heading[4] = '>';    
    string_heading[0] = '<';
    string_heading[4] = '>';    
    u8g2.drawStr(2, 10, string_heading);
        
    // speed
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(44, 12, speed);
    u8g2.setFont(u8g2_font_tinyunicode_tf);
    u8g2.drawStr(49, 18, "MPH");

    // Nav Circles
    u8g2.setDrawColor(1);
    u8g2.drawDisc(25, 35, 24);  // Main Circle
    u8g2.drawDisc(55, 29, 8);   // Wind Circle
    u8g2.setDrawColor(0);
    u8g2.drawDisc(25, 35, 22);  // center empty
    display_drawTrianglePointer(55, 29, wind_angle, 7);
    u8g2.setDrawColor(1);
    u8g2.drawStr(51, 46, windSpeed);
    
    // waypoint name and progress bar
    u8g2.setDrawColor(0);
    u8g2.drawBox(1,49,63, 17);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(1, 49, 63, 4);
    u8g2.drawBox(1, 49, 27, 4);  // 3rd argument is filled width from left (% of 64 pixels)
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(1, 64, waypoint);

    // field dividers
    u8g2.drawLine(1, 65, 64, 65);
    u8g2.drawLine(1, 89, 64, 89);
    u8g2.drawLine(1, 113, 64, 113);
    u8g2.drawLine(33, 65, 33, 113);


    // vario bar
    u8g2.drawLine(52, 114, 52, 192);
    // u8g2.drawFrame(54, 64, 11, 116);
    u8g2.drawBox(53, 164, 12, 20);

    // Time to Waypoint
    u8g2.setFont(leaf_5h);
    //u8g2.drawStr(4, 72, ">TIME");
    u8g2.drawStr(4, 72, "M:S>&");
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(1, 87, timeToWypt);

    // Dist to Waypoint
    u8g2.setFont(leaf_5h);
    //u8g2.drawStr(35, 72, ">KM");
    u8g2.drawStr(35, 72, "KM>%");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(35, 88, distToWypt);

    // Glide over ground now
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(4, 96, "`GLIDE");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(1, 112, glide);

    // Glide to waypoint
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(35, 96, "`WAYPT");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(35, 112, glideToWypt);


    // Timer 
    u8g2.drawBox(7, 116, 42, 16);
    u8g2.setDrawColor(0);
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(14, 130, timer);
    u8g2.setDrawColor(1);

    // Clock
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(5, 147, clockTime);

    // Climb
    u8g2.drawBox(1, 153, 51, 18);
    u8g2.setDrawColor(0);
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(2, 169, climbRate);
    u8g2.setDrawColor(1);
    
    // Altitude(s)
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(4, 177, "ALTITUDE");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(2, 192, altitude);


  } while ( u8g2.nextPage() );  
}
*/

/*

void display_test_real_2() {

  dirToWypt += .005;
  wind_angle -= .0075;
  delay(10);
  u8g2.firstPage();
  do {
    


        
    // speed
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(1, 12, speed);
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(15, 18, "MPH");

    // heading and turn
    u8g2.setFont(leaf_7x10);
    //if (turn == 0) string_heading[0] = '<';
    //if (turn == 1) string_heading[4] = '>';    
    string_heading[0] = '<';
    string_heading[4] = '>';    
    u8g2.drawStr(23, 10, string_heading);

    // Nav Circles
    uint8_t nav_x = 38;
    uint8_t nav_y = 42;
    uint8_t nav_r = 26;
    uint8_t wind_r = 8;

    u8g2.setDrawColor(1);
    u8g2.drawDisc(nav_x, nav_y, nav_r);  // Main Circle
    u8g2.setDrawColor(0);
    u8g2.drawDisc(nav_x, nav_y, nav_r-2);  // center empty
    u8g2.setDrawColor(1);
    u8g2.drawDisc(nav_x, nav_y, wind_r);   // Wind Circle
    
    // Pointer (Travel)
    uint8_t pointer_w = 5;              // half width of arrowhead
    uint8_t pointer_h = 11;             // full height of arrowhead
    uint8_t pointer_x = nav_x;
    uint8_t pointer_y = nav_y-nav_r-4;    //tip of arrow
    u8g2.setDrawColor(0);
    u8g2.drawBox(nav_x-(pointer_w)/2, nav_y-nav_r, pointer_w, 2);
    u8g2.setDrawColor(1);
    // arrow point    
    u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
    u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
    u8g2.drawLine(pointer_x-pointer_w-1, pointer_y+pointer_h, pointer_x-1, pointer_y);
    u8g2.drawLine(pointer_x+pointer_w+1, pointer_y+pointer_h, pointer_x+1, pointer_y);
    //arrow flats
    u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h);
    u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h);
    //arrow shaft
    u8g2.drawLine(pointer_x-pointer_w/2, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h*2);
    u8g2.drawLine(pointer_x+pointer_w/2, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h*2);

    // Waypoint Pointer
    uint8_t waypoint_tip_r = 25;
    uint8_t waypoint_shaft_r = 23;    
    uint8_t waypoint_tail_r = 20;
    float waypoint_arrow_angle = 0.205;
    
    int8_t waypoint_tip_x = sin(dirToWypt)*waypoint_tip_r+nav_x;
    int8_t waypoint_tip_y = nav_y-cos(dirToWypt)*waypoint_tip_r;    
    int8_t waypoint_shaft_x = sin(dirToWypt)*waypoint_shaft_r+nav_x;
    int8_t waypoint_shaft_y = nav_y-cos(dirToWypt)*waypoint_shaft_r;    


    u8g2.drawLine(nav_x+1, nav_y, waypoint_shaft_x+1, waypoint_shaft_y);
    u8g2.drawLine(nav_x, nav_y+1, waypoint_shaft_x, waypoint_shaft_y+1);
    u8g2.drawLine(nav_x, nav_y, waypoint_shaft_x, waypoint_shaft_y);          // the real center line; others are just to fatten it up
    u8g2.drawLine(nav_x-1, nav_y, waypoint_shaft_x-1, waypoint_shaft_y);
    u8g2.drawLine(nav_x, nav_y-1, waypoint_shaft_x, waypoint_shaft_y-1);

    int8_t tail_left_x = sin(dirToWypt - waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
    int8_t tail_left_y = nav_y - cos(dirToWypt - waypoint_arrow_angle) * (waypoint_tail_r);
    int8_t tail_right_x = sin(dirToWypt + waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
    int8_t tail_right_y = nav_y - cos(dirToWypt + waypoint_arrow_angle) * (waypoint_tail_r);

    u8g2.drawLine(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y);
    u8g2.drawLine(tail_right_x, tail_right_y, waypoint_tip_x, waypoint_tip_y);
    u8g2.drawLine(tail_right_x, tail_right_y, tail_left_x, tail_left_y);
    u8g2.drawTriangle(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y, tail_right_x, tail_right_y);




    // Wind Vector
    u8g2.setDrawColor(0);
    display_drawTrianglePointer(nav_x, nav_y, wind_angle, 7);
    u8g2.setDrawColor(1);
    u8g2.setFont(leaf_5x8);
    u8g2.drawStr(53, 19, windSpeed);

    // vario bar    
    u8g2.drawFrame(1, 13, 13, 119);
    u8g2.setDrawColor(0);   
    u8g2.drawLine(12, 15, 12, 58);
    u8g2.setDrawColor(1); 

    // Climb
    u8g2.drawBox(14, 65, 50, 18);
    u8g2.drawTriangle(8, 73, 13, 68, 13, 78);
    u8g2.setDrawColor(0);
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(13, 81, climbRate);
    u8g2.setDrawColor(1);

    // Altitude(s)
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(16, 99, altitude);

    // Clock
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(18, 115, clockTime);

    // Timer 
    uint8_t timer_x = 14;
    uint8_t timer_y = 116;
    uint8_t timer_w = 50;
    uint8_t timer_h = 16;

    u8g2.drawBox(timer_x, timer_y, timer_w, timer_h);
    u8g2.setDrawColor(0);
    
    //u8g2.drawPixel(timer_x, timer_y);
    //u8g2.drawPixel(timer_x, timer_y+timer_h-1);
    //u8g2.drawPixel(timer_x+timer_w-1, timer_y);
    //u8g2.drawPixel(timer_x+timer_w-1, timer_y+timer_h-1);
    
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(timer_x+5, timer_y+timer_h-2, timer);
    u8g2.setDrawColor(1);



    // field dividers
    u8g2.drawLine(14, 101, 64, 101);
    u8g2.drawLine(1, 131, 64, 131);
    u8g2.drawLine(1, 154, 64, 154);
    u8g2.drawLine(1, 177, 64, 177);
    u8g2.drawLine(33, 132, 33, 176);


    // Time to Waypoint
    u8g2.setFont(leaf_5h);    
    u8g2.drawStr(4, 138, "M:S>&");
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(1, 152, timeToWypt);

    // Dist to Waypoint
    u8g2.setFont(leaf_5h);
    //u8g2.drawStr(35, 72, ">KM");
    u8g2.drawStr(35, 138, "KM>%");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(35, 153, distToWypt);

    // Glide over ground now
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(4, 161, "`GLIDE");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(1, 176, glide);

    // Glide to waypoint
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(35, 161, "`WAYPT");
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(35, 176, glideToWypt);



    // waypoint name and progress bar
    u8g2.drawFrame(1, 177, 63, 4);
    u8g2.drawBox(2, 178, 27, 2);  // 3rd argument is filled width from left (% of 64 pixels)
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(1, 192, waypoint);



  } while ( u8g2.nextPage() ); 

}
*/


void display_test_real_3() {

  dirToWypt += .005;
  wind_angle -= .0075;
  delay(10);
  u8g2.firstPage();
  do {        
    // speed
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(1, 12, speed);
    u8g2.setFont(leaf_5h);
    u8g2.drawStr(15, 18, "MPH");

    // heading and turn
    u8g2.setFont(leaf_7x10);
    //if (turn == 0) string_heading[0] = '<';
    //if (turn == 1) string_heading[4] = '>';    
    string_heading[0] = '<';
    string_heading[4] = '>';    
    u8g2.drawStr(23, 10, string_heading);

    // Nav Circles
    uint8_t nav_x = 38;
    uint8_t nav_y = 38;
    uint8_t nav_r = 25;
    uint8_t wind_r = 8;

    u8g2.setDrawColor(1);
    u8g2.drawDisc(nav_x, nav_y, nav_r);  // Main Circle
    u8g2.setDrawColor(0);
    u8g2.drawDisc(nav_x, nav_y, nav_r-2);  // center empty
    u8g2.setDrawColor(1);
    u8g2.drawDisc(nav_x, nav_y, wind_r);   // Wind Circle
    

    // Straight Arrow Pointer (Travel Direction)
    uint8_t pointer_w = 5;              // half width of arrowhead
    uint8_t pointer_h = 9;             // full height of arrowhead
    uint8_t pointer_x = nav_x;
    uint8_t pointer_y = nav_y-nav_r-2;    //tip of arrow

    u8g2.setDrawColor(0);
    u8g2.drawBox(nav_x-(pointer_w)/2, nav_y-nav_r, pointer_w, 2);
    u8g2.setDrawColor(1);
    // arrow point    
    u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
    u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x, pointer_y);
    u8g2.drawLine(pointer_x-pointer_w-1, pointer_y+pointer_h, pointer_x-1, pointer_y);
    u8g2.drawLine(pointer_x+pointer_w+1, pointer_y+pointer_h, pointer_x+1, pointer_y);
    //arrow flats
    u8g2.drawLine(pointer_x-pointer_w, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h);
    u8g2.drawLine(pointer_x+pointer_w, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h);
    //arrow shaft
    u8g2.drawLine(pointer_x-pointer_w/2, pointer_y+pointer_h, pointer_x-pointer_w/2, pointer_y+pointer_h*2);
    u8g2.drawLine(pointer_x+pointer_w/2, pointer_y+pointer_h, pointer_x+pointer_w/2, pointer_y+pointer_h*2);

    // Waypoint Pointer
    uint8_t waypoint_tip_r = 23;
    uint8_t waypoint_shaft_r = 20;    
    uint8_t waypoint_tail_r = 18;
    float waypoint_arrow_angle = 0.205;
    
    int8_t waypoint_tip_x = sin(dirToWypt)*waypoint_tip_r+nav_x;
    int8_t waypoint_tip_y = nav_y-cos(dirToWypt)*waypoint_tip_r;    
    int8_t waypoint_shaft_x = sin(dirToWypt)*waypoint_shaft_r+nav_x;
    int8_t waypoint_shaft_y = nav_y-cos(dirToWypt)*waypoint_shaft_r;    


    u8g2.drawLine(nav_x+1, nav_y, waypoint_shaft_x+1, waypoint_shaft_y);
    u8g2.drawLine(nav_x, nav_y+1, waypoint_shaft_x, waypoint_shaft_y+1);
    u8g2.drawLine(nav_x, nav_y, waypoint_shaft_x, waypoint_shaft_y);          // the real center line; others are just to fatten it up
    u8g2.drawLine(nav_x-1, nav_y, waypoint_shaft_x-1, waypoint_shaft_y);
    u8g2.drawLine(nav_x, nav_y-1, waypoint_shaft_x, waypoint_shaft_y-1);

    int8_t tail_left_x = sin(dirToWypt - waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
    int8_t tail_left_y = nav_y - cos(dirToWypt - waypoint_arrow_angle) * (waypoint_tail_r);
    int8_t tail_right_x = sin(dirToWypt + waypoint_arrow_angle) * (waypoint_tail_r) + nav_x;
    int8_t tail_right_y = nav_y - cos(dirToWypt + waypoint_arrow_angle) * (waypoint_tail_r);

    u8g2.drawLine(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y);
    u8g2.drawLine(tail_right_x, tail_right_y, waypoint_tip_x, waypoint_tip_y);
    u8g2.drawLine(tail_right_x, tail_right_y, tail_left_x, tail_left_y);
    u8g2.drawTriangle(tail_left_x, tail_left_y, waypoint_tip_x, waypoint_tip_y, tail_right_x, tail_right_y);




    // Wind Vector
    u8g2.setDrawColor(0);
    display_drawTrianglePointer(nav_x, nav_y, wind_angle, 7);
    u8g2.setDrawColor(1);
    u8g2.setFont(leaf_5x8);
    u8g2.drawStr(53, 19, windSpeed);

    // vario bar    
    u8g2.drawFrame(1, 13, 13, 110);
    //u8g2.setDrawColor(0);       //only needed if nav cricle overlaps vario bar
    //u8g2.drawLine(12, 15, 12, 58);
    u8g2.setDrawColor(1); 

    uint8_t cursor_y = 57;

    // Climb    
    u8g2.drawBox(14, cursor_y, 50, 18);
    u8g2.drawTriangle(8, cursor_y+8, 13, cursor_y+3, 13, cursor_y+13);
    u8g2.setDrawColor(0);
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(13, cursor_y+16, climbRate);
    u8g2.setDrawColor(1);
    cursor_y += 18;

    // Altitude(s)
    u8g2.setFont(leaf_8x14);
    u8g2.drawStr(16, cursor_y+16, altitude);
    cursor_y += 16;

    // Line divider
    u8g2.drawLine(14, cursor_y+2, 64, cursor_y+2);
    cursor_y += 2;

    // Clock
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(18, cursor_y+14, clockTime);
    cursor_y +=15;

    // Timer 
    uint8_t timer_x = 14;
    uint8_t timer_y = cursor_y;
    uint8_t timer_w = 50;
    uint8_t timer_h = 15;
    u8g2.drawBox(timer_x, timer_y, timer_w, timer_h);
    u8g2.setDrawColor(0);    
    //u8g2.drawPixel(timer_x, timer_y);
    //u8g2.drawPixel(timer_x, timer_y+timer_h-1);
    //u8g2.drawPixel(timer_x+timer_w-1, timer_y);
    //u8g2.drawPixel(timer_x+timer_w-1, timer_y+timer_h-1);    
    u8g2.setFont(leaf_6x12);
    u8g2.drawStr(timer_x+5, timer_y+timer_h-1, timer);
    u8g2.setDrawColor(1);
    cursor_y += 15;

    //***** FIELD BOXES *****//

    uint8_t temp_cursor = cursor_y;     //save cursor position for next column

    // Time to Waypoint
    u8g2.setFont(leaf_5h);    
    cursor_y += 6;
    u8g2.drawStr(1, cursor_y, "TIME>%");    
    u8g2.setFont(leaf_6x12);
    cursor_y +=13;
    u8g2.drawStr(1, cursor_y, timeToWypt);

    cursor_y = temp_cursor;

    // Dist to Waypoint
    u8g2.setFont(leaf_5h);
    cursor_y += 6;
    u8g2.drawStr(35, cursor_y, "KM>&");
    //u8g2.drawStr(35, 138, "KM>%");
    u8g2.setFont(leaf_6x12);
    cursor_y += 13;
    u8g2.drawStr(38, cursor_y, distToWypt);

    // column divider
    u8g2.drawLine(33, temp_cursor, 33, cursor_y);

    // divider line
    cursor_y += 1;
    u8g2.drawLine(1, cursor_y, 64, cursor_y);
    cursor_y += 1;

    temp_cursor = cursor_y;             //save cursor position for next column

    // Glide over ground now
    u8g2.setFont(leaf_5h);
    cursor_y += 6;
    u8g2.drawStr(1, cursor_y, "`GLIDE");
    u8g2.setFont(leaf_6x12);
    cursor_y += 13;
    u8g2.drawStr(4, cursor_y, glide);

    cursor_y = temp_cursor;

    // Glide to waypoint
    u8g2.setFont(leaf_5h);
    cursor_y += 6;
    u8g2.drawStr(35, cursor_y, "`WAYPT");
    u8g2.setFont(leaf_6x12);
    cursor_y += 13;
    u8g2.drawStr(38, cursor_y, glideToWypt);

    // column divider
    u8g2.drawLine(33, temp_cursor, 33, cursor_y);

    // divider line
    cursor_y += 1;
    u8g2.drawLine(1, cursor_y, 64, cursor_y);
    cursor_y += 1;



    // waypoint name and progress bar
    u8g2.drawFrame(1, cursor_y-1, 63, 4);
    u8g2.drawBox(2, 178, 27, 2);  // 3rd argument is filled width from left (% of 64 pixels)
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(1, cursor_y+11, waypoint);

    //icons
    u8g2.setFont(leaf_icons);
    u8g2.drawStr(1,192,"6");
    u8g2.drawStr()



  } while ( u8g2.nextPage() ); 

}

void display_drawTrianglePointer(uint16_t x, uint16_t y, float angle, uint16_t radius) {    
    if (angle > 2 * PI) angle = 0;

    int16_t wind_triangle_tip_len = radius-1;  // pixels from center for point
    int16_t wind_triangle_tail_len = radius-1;  // pixels from center for the tails
    float wind_triangle_tail_angle = 0.65;  // pixels tail width (half-width)
    /*
    u8g2.setDrawColor(0);
    u8g2.drawDisc(wind_triangle_center_x, wind_triangle_center_y, wind_triangle_radius);
    u8g2.setDrawColor(1);
    u8g2.drawCircle(wind_triangle_center_x, wind_triangle_center_y, wind_triangle_radius);        
    */
    uint16_t tip_xprime = x + sin(angle + PI)*wind_triangle_tip_len;
    uint16_t tip_yprime = y - cos(angle + PI)*wind_triangle_tip_len;
    uint16_t tail_1_xprime = x + sin(angle + wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_1_yprime = y - cos(angle + wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_2_xprime = x + sin(angle - wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_2_yprime = y - cos(angle - wind_triangle_tail_angle)*wind_triangle_tail_len;
    uint16_t tail_mid_xprime = x + sin(angle)*wind_triangle_tail_len/2;
    uint16_t tail_mid_yprime = y - cos(angle)*wind_triangle_tail_len/2;
    
    u8g2.drawTriangle(tip_xprime, tip_yprime, tail_1_xprime, tail_1_yprime, tail_mid_xprime, tail_mid_yprime);
    u8g2.drawTriangle(tip_xprime, tip_yprime, tail_2_xprime, tail_2_yprime, tail_mid_xprime, tail_mid_yprime);
    u8g2.drawLine(tip_xprime, tip_yprime, tail_1_xprime, tail_1_yprime);
    u8g2.drawLine(tail_mid_xprime, tail_mid_yprime, tail_2_xprime, tail_2_yprime);
}



// Draw polygons 
// by default, u8g2 lib supports polygons to 6 points, but only exposes u8g2_DrawTriangle
// Max polygon point size can be increased from 6 in the u8g2 library, but we'll try to write display code that doesn't need library modifications
/*
void u8g2_DrawPoly(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  u8g2_DrawPoly(x0, y0, x1, y1, x2, y2, false, false, false, false, false, false);  
}

void u8g2_DrawPoly(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3) {
  u8g2_DrawPoly(x0, y0, x1, y1, x2, y2, x3, y3, false, false, false, false);  
}

void u8g2_DrawPoly(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t x4, uint16_t y4) {
  u8g2_DrawPoly(x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, false, false);  
}

void u8g2_DrawPoly(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t x4, uint16_t y4, uint16_t x5, uint16_t y5) {
  u8g2_ClearPolygonXY();
  u8g2_AddPolygonXY(u8g2, x0, y0);
  u8g2_AddPolygonXY(u8g2, x1, y1);
  u8g2_AddPolygonXY(u8g2, x2, y2);
  if(x3) {
    u8g2_AddPolygonXY(u8g2, x3, y3);
    if(x4) {
      u8g2_AddPolygonXY(u8g2, x4, y4);
      if(x5) {
        u8g2_AddPolygonXY(u8g2, x5, y5);  
      }
    }
  }
  u8g2_DrawPolygon(u8g2);
}
*/


//void rotate_points()

