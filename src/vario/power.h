/*
 * power.h
 * 
 * Battery Charging Chip BQ24073
 * 
 */
#ifndef power_h
#define power_h

#include <Arduino.h>

//Pinout for Leaf V3.2.0
#define POWER_CHARGE_I1   39
#define POWER_CHARGE_I2   40
#define POWER_LATCH       48  
#define POWER_CHARGE_GOOD 47  // INPUT
#define BATT_SENSE         1  // INPUT ADC

#define BATT_FULL_MV     4150  // mV full battery on which to base % full (100%)
#define BATT_EMPTY_MV    3250  // mV empty battery on which to base % full (0%)
#define BATT_SHUTDOWN_MV 3175  // mV at which to shutdown the system to prevent battery over-discharge 
//Note: the battery also has over discharge protection, but we don't fully trust it, plus we want to shutdown while we have power to save logs etc


enum power_on_states {ON_USB_CHARGE, ON_BATT_ONLY, ON_USB_NO_BATT};
enum power_input_levels {i100mA, i500mA, iMax, iStandby}; 
// iMax set by ILIM pin resistor on battery charger chip. Results in 1.348Amps max input (for battery charging AND system load)
// Note: with this higher input limit, the battery charging will then be limited by the ISET pin resistor value, to approximately 810mA charging current)

uint8_t power_init(void);
void power_simple_init(void);

void power_turn_on(void);
void power_turn_off(void);

void power_set_input_current(uint8_t current);
uint32_t power_get_batt_level(bool pct);

void power_test(void);

#endif