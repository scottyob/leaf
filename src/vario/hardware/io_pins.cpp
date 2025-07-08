#include "hardware/io_pins.h"
#include <TCA9555.h>
#include <Wire.h>

// First try to load up any config from variants
#include "io_pins.h"
#include "variant.h"

// Pin configuration for the IO Expander (0=output 1=input)
#ifndef IOEX_REG_CONFIG_PORT0
#define IOEX_REG_CONFIG_PORT0 0b11111111  // default all inputs
#endif

#ifndef IOEX_REG_CONFIG_PORT1
#define IOEX_REG_CONFIG_PORT1 0b11111111  // default all inputs
#endif

volatile unsigned long ioexLastInterruptMicros = 0;

// Create IO Expander
TCA9535 IOEX(IOEX_ADDR);

void ioexInit() {
  // start IO Expander
  bool result = IOEX.begin();
  Serial.print("IOEX.begin succes: ");
  Serial.println(result);

  // configure IO expander pins
  result = IOEX.pinMode8(0, IOEX_REG_CONFIG_PORT0);
  Serial.print("IOEX.pinModeP0 succes: ");
  Serial.println(result);
  IOEX.pinMode8(1, IOEX_REG_CONFIG_PORT1);
  Serial.print("IOEX.pinModeP1 succes: ");
  Serial.println(result);

  // Attach the interrupt handler for the IO Expander
  attachInterrupt(IOEX_INTERRUPT_PIN, onIoexInterrupt, RISING);
}

void ioexDigitalWrite(bool onIOEX, uint8_t pin, uint8_t value) {
  // if we're writing a pin on the IO Expander
  if (onIOEX) {
    bool result = IOEX.write1(pin, value);
  } else {
    digitalWrite(pin, value);
  }
}

unsigned long ioexLastInterruptMs() {
  // Return the last time the IO Expander was interrupted
  return micros() - ioexLastInterruptMicros / 1000;  // Convert to milliseconds
}

void IRAM_ATTR onIoexInterrupt() { ioexLastInterruptMicros = micros(); }