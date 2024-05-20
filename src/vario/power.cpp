#include "power.h"


uint8_t power_init(void) {

  // Set output / input pins to control battery charge and power supply  
  pinMode(POWER_LATCH, OUTPUT);
  pinMode(POWER_CHARGE_I1, OUTPUT);
  pinMode(POWER_CHARGE_I2, OUTPUT);  
  pinMode(POWER_CHARGE_GOOD, INPUT);
  pinMode(BATT_SENSE, INPUT);

  
  Serial.print("power_init ADC check: ");
  Serial.print(analogRead(BATT_SENSE));
  Serial.println("; completed");


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


uint32_t ADC_value;

uint32_t power_get_batt_level(bool pct) {
  Serial.println("starting ADC read");
  ADC_value = analogRead(BATT_SENSE);
  Serial.println(ADC_value);
  delay(100);
  uint32_t batt_level_mv = ADC_value /*analogRead(BATT_SENSE)*/ * 5554 / 4095;  //    (3300mV ADC range / .5942 V_divider) = 5554.  Then divide by 4095 steps of resolution
  uint32_t batt_level_pct;
  Serial.println("Calculating pct");
  if (batt_level_mv < BATT_EMPTY_MV) {
    batt_level_pct = 0;
  } else if (batt_level_mv > BATT_FULL_MV) {
    batt_level_pct = 100;
  } else {
    batt_level_pct = 100 * (batt_level_mv - BATT_EMPTY_MV) / (BATT_FULL_MV - BATT_EMPTY_MV);
  }  
  Serial.println("returning");
  if (pct) return batt_level_pct;  
  else return batt_level_mv;     
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

void power_test(void) {
  Serial.print("Batt Voltage: ");
  Serial.print(power_get_batt_level(0));
  Serial.print(", Batt Percent: ");
  Serial.print(power_get_batt_level(1));
  Serial.println("%");
  delay(500);
}