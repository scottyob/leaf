# Leaf Hardware
Brief summary of hardware changes

1. [PCBA versions and changes](#PCBA)
2. [Plastic Case](#plastic-case)
3. [Battery](#battery)

## PCBA
### v3.2.5

#### Changes
+ Replace bare ESP32S3 with full module (ESP32-S3-MINI-1).  Can utilize Espressif's FCC and CE certifications
+ Add optional LoRa module (Wio-SX1262 by SeeedStudio).  NOTE: with LoRa module installed, battery moves forward in position by 3mm compared to 3.2.4.  May not fit in case design v3.2.4.
+ Add IO Expander for 16 more GPIO pins
+ Adjust USB-EN voltage divider (used for signaling HI on power_enable and also GPS backup power when no battery).  Also connected USB-EN to GPS Backup Regulator, instead of GPS directly.  These changes allow for higher input voltage on USB connector (now limit is 9V vs previous 6V).  
+ Moved all header pins to one edge (out of the way of ESP module antenna)
+ Moved Charge, VCC 3.3 circuitry and battery connector to make more room for battery
+ Other minor changes


#### ESP32 Pinout Changes
| ESP32       |   3.2.5 Function   |     Previous 3.2.4 Function     |
|--------|-----------|-----------------|
|GPIO   0|LORA_RESET |AVAIL_GPIO_0  (gone)   |
|GPIO   7|LORA_RF_SW |IMU_INT / GPIO_7 |
|GPIO  15|LORA_DIO1  |SPKR_VOL_A       |
|GPIO  16|LORA_BUSY  |SPKR_VOL_B       |
|GPIO  39|IO_EX_INT  |AVAIL_GPIO_39    |
|GPIO  46|LORA_CS    |GPS_1PPS         |

#### LoRa SX1262 Comms Module
Seeed Studio WIO-SX1262
[info & datasheet near bottom of page](https://www.seeedstudio.com/Wio-SX1262-Wireless-Module-p-5981.html)

#### IO Expander 
[datasheet](https://wmsc.lcsc.com/wmsc/upload/file/pdf/v2/lcsc/2304251416_XINLUDA-XL9535QF24_C5444301.pdf)
Chip is configured with I2C address: 0x20

|IOEX Pin# |Function
|--------|--------|
|P00|GPS_1PPS (input)
|P01|SPKR_VOL_B           
|P02|SPKR_VOL_A     
|P03|EYESPI_SDcard_CS     
|P04|No Connection
|P05|EYESPI_GPIO_2 
|P06|EYESPI_GPIO_1  
|P07|EYESPI_BUSY (input)  
|P10|EYESPI_INT (input)
|P11|EYESPI_MEM_CS     
|P12|EYESPI_TS_CS  
|P13|IMU_INT (input)
|P14|EX_14 *Expanded GPIO
|P15|EX_15 *Expanded GPIO
|P16|EX_16 *Expanded GPIO
|P17|EX_17 *Expanded GPIO

(*EYESPI is the multipurpose AdaFruit 18-pin connector ("Display 2"))


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