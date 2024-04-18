## Required libraries

* FastLED: 3.6.0
* TinyGPSPlus-ESP32
* U8g2 LCD: 2.34.22




tried but not going to use:
* NeoGPS: 4.2.9
    -> change GPSport.h to define HWSerial:
          #define gpsPort Serial0
          #define GPS_PORT_NAME "Serial0"
          #define DEBUG_PORT Serial
    -> enable PARSE_GSV string and PARSE_SATELLITES and PARSE_SATELLITE_INFO
    -> enable NMEAGPS_INTERRUPT_PROCESSING

