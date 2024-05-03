/*
 * IMU.cpp
 *
 * TDK InvenSense ICM-20498
 * 6 DOF Gyro+Accel plus 3-axis mag
 *
 */
#include "IMU.h"

#define address_WHO_AM_I  0
#define address_USER_CTRL 3
#define address_PWR_MGMT_1 6
#define address_PWR_MGMT_2 7
#define address_REG_BANK_SEL 127


#define default_USER_CTRL   0b01010000   // reset I2C and put into SPI mode
#define default_PWR_MGMT_1  0b00000100   // reset, wake up, low power off, temp on, clock '4' (auto select best source)
#define default_PWR_MGMT_2  0b00000000   // accel on, gyro on
#define select_bank_0       0b00000000   // bank 0
#define select_bank_1       0b00010000   // bank 1
#define select_bank_2       0b00100000   // bank 2
#define select_bank_3       0b00110000   // bank 3


void imu_init(void) {

  delay(100);
  imu_spiWrite(address_REG_BANK_SEL, select_bank_0);
  delay(20);
  imu_spiWrite(address_USER_CTRL, default_USER_CTRL);
  delay(20);
  imu_spiWrite(address_PWR_MGMT_1, default_PWR_MGMT_1);
  delay(20);
  imu_spiWrite(address_PWR_MGMT_2, default_PWR_MGMT_2);
  delay(20);

  Serial.print("who am I: ");
  Serial.println(imu_spiRead(address_WHO_AM_I));

  Serial.print("user control: ");
  Serial.println(imu_spiRead(address_USER_CTRL));

  Serial.print("Power Management 1: ");
  Serial.println(imu_spiRead(address_PWR_MGMT_1));

  Serial.print("Power Management 2: ");
  Serial.println(imu_spiRead(address_PWR_MGMT_2));
    
}

void imu_test(void) {
  delay(100);
  for(int i = 45; i<51; i++) {
    Serial.print(imu_spiRead(i));        
    Serial.print(" ");  
  }
  Serial.println("");
}


