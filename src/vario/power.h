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

#define BATT_FULL_MV     4080  // mV full battery on which to base % full (100%)
#define BATT_EMPTY_MV    3250  // mV empty battery on which to base % full (0%)
#define BATT_SHUTDOWN_MV 3200  // mV at which to shutdown the system to prevent battery over-discharge 
//Note: the battery also has over discharge protection, but we don't fully trust it, plus we want to shutdown while we have power to save logs etc

enum power_on_states {  
  POWER_OFF,       // power off we'll never use, because chip is unpowered and not running
  POWER_ON,        // system is ON by user input (pushbutton) and should function normally.  Dislpay battery icon and charging state depending on what charger is doing (we can't tell if USB is plugged in or not, only if battery is charging or not)
  POWER_OFF_USB    // system is OFF, but has USB power.  Keep power usage to a minimum, and just power on display to show battery charge state (when charging) or turn display off and have processor sleep (when not charging)
};                 //   ... note: we enter POWER_OFF_USB state either from POWER_OFF and then plugging in to USB power (no power button detected during boot) or from POWER_ON with USB plugged in, and user turning off power via pushbutton.

enum power_input_levels {i100mA, i500mA, iMax, iStandby}; 

// iMax set by ILIM pin resistor on battery charger chip. Results in 1.348Amps max input (for battery charging AND system load)
// Note: with this higher input limit, the battery charging will then be limited by the ISET pin resistor value, to approximately 810mA charging current)

extern uint8_t powerOnState;

void power_bootUp(void);
void power_init(void);
void power_latch_on(void);
void power_latch_off(void);
void power_shutdown(void);

void power_init_peripherals(void);  
void power_sleep_peripherals(void);
void power_wake_peripherals(void);
void power_switchToOnState(void);

void power_update(void);

void power_set_input_current(uint8_t current);
uint16_t power_getBattLevel(uint8_t value);
bool power_getBattCharging(void);
uint8_t power_getInputCurrent(void);

//testing
  void power_test(void);
  void power_simple_init(void);
#endif