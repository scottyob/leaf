#include <Arduino.h>

// include the library
#include <SPI.h>

#define RADIOLIB_DEBUG_BASIC \
  (1)  // basic debugging (e.g. reporting GPIO timeouts or module not being found)
#define RADIOLIB_DEBUG_PROTOCOL (1)  // protocol information (e.g. LoRaWAN internal information)
#define RADIOLIB_DEBUG_SPI \
  (1)  // verbose transcription of all SPI communication - produces large debug logs!
#define RADIOLIB_VERBOSE_ASSERT \
  (1)  // verbose assertions - will print out file and line number on failure
#define RADIOLIB_DEBUG_PORT Serial

#include <RadioLib.h>

// ##### Hardware Platform Specific Parameters
// ### Leaf Dev board

#define LORA_CHIP SX1262_CHIP
// DIO2 is connected to antenna switch (TX_EN, common for a lot of libraries)
#define LORA_USE_DIO2_ANT_SWITCH 1
#define LORA_PIN_LORA_RESET 39     // LORA RESET
#define LORA_PIN_LORA_DIO_1 46     // LORA DIO_1
#define LORA_PIN_LORA_BUSY 7       // LORA BUSY
#define LORA_PIN_LORA_NSS 0        // LORA SPI CS
#define LORA_PIN_LORA_SCLK 12      // LORA SPI CLK
#define LORA_PIN_LORA_MISO 13      // LORA SPI MISO
#define LORA_PIN_LORA_MOSI 11      // LORA SPI MOSI
#define LORA_RADIO_TXEN -1         // LORA ANTENNA TX ENABLE
#define LORA_RADIO_RXEN 21         // LORA ANTENNA RX ENABLE
#define LORA_DIO2_ANT_SWITCH true  // We use DIO2 as an antenna switch

// SPI SETUP
// SPI and peripheral devices using the default SPI Bus (SPI2 / FSPI)
#define SPI_MOSI 11
#define SPI_CLK 12
#define SPI_MISO 13

#define SPI_SS_LCD 15

// Lora objects
// NSS, DIO1, NRST, BUSY
SX1262 radio = new Module((uint32_t)LORA_PIN_LORA_NSS,
                          (uint32_t)LORA_PIN_LORA_DIO_1,
                          (uint32_t)LORA_PIN_LORA_RESET,
                          (uint32_t)LORA_PIN_LORA_BUSY);

// flag to indicate that a packet was received
// As this can be updated from within an interrupt, we use a volatile bool
// to let the compiler know that this can't be optimized and the value will change
// outside of the main runloop.
volatile bool receivedFlag = false;

// This function is to be called from an interrupt to flag that we've had a packet
// received and needs to be processed.  The RAM_ATTR attribute will ensure that this
// function lives in memory and not flash, a requirement for callbacks from interrupts
ICACHE_RAM_ATTR
void setFlag() {
  receivedFlag = true;
}

void SetupLora();

void setup() {
  Serial.begin(9600);
  delay(3000);
  SetupLora();
}

void SetupLora() {
  pinMode(SPI_SS_LCD, OUTPUT);
  digitalWrite(SPI_SS_LCD, HIGH);
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);

  // initialize SX1268 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) {
      delay(10);
    }
  }

  // set the function that will be called
  // when new packet is received
  radio.setPacketReceivedAction(setFlag);

  // set carrier frequency to US Meshtastic 906.875.  In the US, frequency slot 20.
  if (radio.setFrequency(906.875) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set bandwidth to 250 kHz
  if (radio.setBandwidth(250.0) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set spreading factor to 11 (Meshtastic longFast)
  if (radio.setSpreadingFactor(11) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set coding rate to 5 (Meshtastic longFast, coding rate 4/5)
  if (radio.setCodingRate(5) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set LoRa sync word to 0x2B for Meshtastic
  if (radio.setSyncWord(0x2B) != RADIOLIB_ERR_NONE) {
    Serial.println(F("Unable to set sync word!"));
    while (true) {
      delay(10);
    }
  }

  // set output power to 10 dBm (accepted range is -17 - 22 dBm)
  if (radio.setOutputPower(10) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
  // NOTE: set value to 0 to disable overcurrent protection
  if (radio.setCurrentLimit(80) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    Serial.println(F("Selected current limit is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // set LoRa preamble length to 16 symbols (accepted range is 0 - 65535)
  // 16 is for Meshtastic
  if (radio.setPreambleLength(16) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  //   // disable CRC
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    Serial.println(F("Selected CRC is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // Some SX126x modules have TCXO (temperature compensated crystal
  // oscillator). To configure TCXO reference voltage,
  // the following method can be used.
  //   if (radio1.setTCXO(2.4) == RADIOLIB_ERR_INVALID_TCXO_VOLTAGE) {
  //     Serial.println(F("Selected TCXO voltage is invalid for this module!"));
  //     while (true) {
  //       delay(10);
  //     }
  //   }

  // Some SX126x modules use DIO2 as RF switch. To enable
  // this feature, the following method can be used.
  // NOTE: As long as DIO2 is configured to control RF switch,
  //       it can't be used as interrupt pin!
  if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
    Serial.println(F("Failed to set DIO2 as RF switch!"));
    while (true) {
      delay(10);
    }
  }

  Serial.println(F("All settings succesfully changed!"));

  // start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) {
      delay(10);
    }
  }
}

void loop() {
  // check if the flag is set
  if (receivedFlag) {
    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);

    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int numBytes = radio.getPacketLength();
      int state = radio.readData(byteArr, numBytes);
    */

    if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(str);

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1262] RSSI:\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1262] SNR:\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // print frequency error
      Serial.print(F("[SX1262] Frequency error:\t"));
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
  }
}