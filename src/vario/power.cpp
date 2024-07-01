#include "power.h"



void power_simple_init(void) {
  
  #define POWER_CHARGE_I1   39
  #define POWER_CHARGE_I2   40
  #define POWER_LATCH       48  

  // enable 3.3V regulator
  pinMode(POWER_LATCH, OUTPUT);
  digitalWrite(POWER_LATCH, HIGH);

  // set power supply to 500mA mode
  pinMode(POWER_CHARGE_I1, OUTPUT);
  pinMode(POWER_CHARGE_I2, OUTPUT);  
  digitalWrite(POWER_CHARGE_I1, HIGH);
  digitalWrite(POWER_CHARGE_I2, LOW);
}


uint8_t power_init(void) {

  // Set output / input pins to control battery charge and power supply  
  pinMode(POWER_LATCH, OUTPUT);
  pinMode(POWER_CHARGE_I1, OUTPUT);
  pinMode(POWER_CHARGE_I2, OUTPUT);  
  pinMode(POWER_CHARGE_GOOD, INPUT_PULLUP);
  pinMode(BATT_SENSE, INPUT);

  // configure system for power-up and initialization
    
  // check power on state
  char powerOnState;
  // plugged in?
  // battery present?
  // charging?
  // return {ON_UNKNOWN, ON_BATT_ONLY, ON_USB_CHARGE, ON_USB_NO_BATT};

  power_turn_on();

  return ON_USB_NO_BATT;
}

void power_update() {
  //TODO: fill this in
  // stuff like checking batt state, checking auto-off, etc
}



uint32_t ADC_value;

uint16_t power_getBattLevel(uint8_t value) {
  uint16_t adc_level = analogRead(BATT_SENSE);
  //uint16_t batt_level_mv = adc_level * 5554 / 4095;  //    (3300mV ADC range / .5942 V_divider) = 5554.  Then divide by 4095 steps of resolution
  uint16_t batt_level_mv = adc_level * 5300 / 4095 + 260; // adjusted formula to account for ESP32 ADC non-linearity; based on calibration measurements.  This is most accurate between 2.4V and 4.7V
  uint16_t batt_level_pct;
  if (batt_level_mv < BATT_EMPTY_MV) {
    batt_level_pct = 0;
  } else if (batt_level_mv > BATT_FULL_MV) {
    batt_level_pct = 100;
  } else {
    batt_level_pct = 100 * (batt_level_mv - BATT_EMPTY_MV) / (BATT_FULL_MV - BATT_EMPTY_MV);
  }  
  if (value == 1) return batt_level_mv;  
  else if (value == 2) return adc_level;
  else return batt_level_pct;
}

bool power_getBattCharging() {
  return (!digitalRead(POWER_CHARGE_GOOD)); // logic low is charging, logic high is not
}

// Note: the Battery Charger Chip has controllable input current (which is then used for both batt charging AND system load).  The battery will be charged with whatever current is remaining after system load.

void power_set_input_current(uint8_t current) {
  switch (current) {
    default:
    case i100mA:
      digitalWrite(POWER_CHARGE_I1, LOW);
      digitalWrite(POWER_CHARGE_I2, LOW);
      break;
    case i500mA:
      digitalWrite(POWER_CHARGE_I1, HIGH);
      digitalWrite(POWER_CHARGE_I2, LOW);
      break;
    case iMax:      // Approx 1.348A max inpuyt current (set by ILIM pin resistor value).
                    // In this case, battery charging will then be limited by the max fast-charge limit set by the ISET pin resistor value (approximately 810mA to the battery).
      digitalWrite(POWER_CHARGE_I1, LOW);
      digitalWrite(POWER_CHARGE_I2, HIGH);
      break;
    case iStandby:  // USB Suspend mode - turns off charger (and system power?) (TODO: check what this actually does)
      digitalWrite(POWER_CHARGE_I1, HIGH);
      digitalWrite(POWER_CHARGE_I2, HIGH);
      break;
  }

}

void power_turn_on(void) {
  digitalWrite(POWER_LATCH, HIGH);
  power_set_input_current(i500mA);
}

void power_turn_off(void) {
  // TODO: all the pre-shutdown operations like disabling devices and saving logs and system data
  digitalWrite(POWER_LATCH, LOW);   // disable 3.3V regulator.  Systems will immediately lose power and shut down processor (after user lets go of center button)
}

void power_test(void) {

  Serial.print("Batt Voltage: ");
  Serial.print(power_getBattLevel(0));
  Serial.print(", Batt Percent: ");
  Serial.print(power_getBattLevel(1));
  Serial.print("%, Charge_Good: ");
  if (!digitalRead(POWER_CHARGE_GOOD)) Serial.println("Charging!");
  else Serial.println("Not Charging!");

  if (Serial.available() > 0) {
    char letter = char(Serial.read());
    while (Serial.available() > 0) {
      Serial.read(); //empty buffer
    }

    int fx = 0;
    switch (letter) {
      case '1': power_set_input_current(i100mA); Serial.println("Input Current to 100mA"); break;
      case '2': power_set_input_current(i500mA); Serial.println("Input Current to 500mA"); break;
      case '3': power_set_input_current(iMax); Serial.println("Input Current to Max (1348mA input; ~810mA charger)"); break;
      case '0': power_set_input_current(iStandby); Serial.println("Input Current to Standby"); break;
    }
  } 
  delay(500);
}