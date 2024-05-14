#include "power.h"




//Pinout for Leaf V3.2.0
#define POWER_CHARGE_I1   39
#define POWER_CHARGE_I2   40
#define POWER_LATCH       48  
#define POWER_CHARGE_GOOD 47  // INPUT
#define BATT_SENSE         1  // INPUT ADC

uint8_t power_init(void) {

  // Set output / input pins to control battery charge and power supply  
  pinMode(POWER_LATCH, OUTPUT);
  pinMode(POWER_CHARGE_I1, OUTPUT);
  pinMode(POWER_CHARGE_I2, OUTPUT);  
  pinMode(POWER_CHARGE_GOOD, INPUT);
  pinMode(BATT_SENSE, INPUT);

  // configure system for power-up and initialization
  // etc etc

  // check power on state
  char powerOnState;
  // plugged in?
  // battery present?
  // charging?
  // return {ON_UNKNOWN, ON_BATT_ONLY, ON_USB_CHARGE, ON_USB_NO_BATT};

  return ON_USB_NO_BATT;
}



uint32_t power_get_batt_level(void) {
  uint32_t batt_level_mv = analogRead(BATT_SENSE) * 5217 / 4095;  // 3100mV ADC range / .5942 V_divider = 5217.  Then divide by 4095 steps of resolution
  uint32_t batt_level_pct;
  if (batt_level_mv < BATT_EMPTY_MV) {
    batt_level_pct = 0;
  } else if (batt_level_mv > BATT_FULL_MV) {
    batt_level_pct = 100;
  } else {
    batt_level_pct = (batt_level_mv - BATT_EMPTY_MV) / (BATT_FULL_MV - BATT_EMPTY_MV);
  }  
  
  //return batt_level_pct;  
  return batt_level_mv;     
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
  digitalWrite(POWER_LATCH, LOW);   // disable 3.3V regulator.  Systems will immediately lose power and shut down processor
}

