# Leaf Hardware
Brief summary of hardware changes

## PCBA
### v3.2.5
+ Replace bare ESP32S3 with full module (ESP32-S3-MINI-1).  Can utilize Espressif's FCC and CE certifications
+ Add optional LoRa module (Wio-SX1262 by SeeedStudio).  NOTE: with LoRa module installed, battery moves forward in position compared to 3.2.4.  May not fit in case design v3.2.4.
+ Adjust USB-EN voltage divider (used for signalling HI on power_enable and also GPS backup power when no battery).  Allows for higher input voltage on USB connector (now limit is 9V vs previous 6V).  Also connected USB-EN to GPS Backup Regulator, instead of GPS directly.
+ Moved all header pins to one edge (out of the way of ESP module antenna)
+ Moved Charge, VCC 3.3 circuitry and battery connector to make more room for battery
+ Other minor changes
### v3.2.4
+ Fix: insufficient current to power GPS backup supply from USB cable (when no battery attached)
+ Fix: silkscreen errors
### v3.2.3
+ Change to superior 12-pin LCD display (192x96px)
+ Add additional "eye-spi" LCD connector for any AdaFruit and other 3rd party displays
+ improved header pins along edges and added notches for backlight version of LCD
### v3.2.2
+ 14-pin 96px wide LCD display
+ Add I2C expansion connector
+ Add Temp/Humidty sensor
+ Boot/Reset switches moved underneath speaker grille
### v3.2.0
+ First version with new ESP32 S3 processor
+ 10-pin 64px wide LCD display

## Plastic Case
### v3.2.4
+ Increased 'flares' of main button for easier grip
+ Increased 'clip force' in baseplate
+ Baseplate can use same width velcro as Leaf
+ Stiffening webs added to battery tabs
+ Screw bosses adjusted for proper M2x8mm thread-forming screws
### v3.2.0
+ Initial case design
+ Alternative-Configuration files available to support additional display types (TFT color LCD, FSTN with backlight, etc)

## Battery
+ Standard size 103450 single cell lipo (3.7V).  10mm x 34mm x 50mm