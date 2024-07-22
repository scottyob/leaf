## Required libraries

* TinyGPSPlus-ESP32 0.0.2
* U8g2 LCD: 2.34.22
* SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library: 1.2.12

## ESP32 configuration
 ESP Board Manager Package 3.0.2
* USB CDC On Boot -> "Enabled"
* Flash Size -> 8MB
* Partition Scheme -> 8M with spiffs (3MB APP/1.5MB SPIFFS)


tried but not going to use:
* NeoGPS: 4.2.9
    -> change GPSport.h to define HWSerial:
          #define gpsPort Serial0
          #define GPS_PORT_NAME "Serial0"
          #define DEBUG_PORT Serial
    -> enable PARSE_GSV string and PARSE_SATELLITES and PARSE_SATELLITE_INFO
    -> enable NMEAGPS_INTERRUPT_PROCESSING
* ICM20948_WE
* AdaFruit ICM20948 (and associated dependencies)