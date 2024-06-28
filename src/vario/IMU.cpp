/*
 * IMU.cpp
 *
 * TDK InvenSense ICM-20498
 * 6 DOF Gyro+Accel plus 3-axis mag
 *
 */
#include "IMU.h"
#include "Leaf_SPI.h"

void imu_init(void) {

  pinMode(IMU_INTERRUPT, INPUT);

  delay(100);
  spi_writeIMUByte(address_REG_BANK_SEL, select_bank_0);
  delay(20);
  spi_writeIMUByte(address_USER_CTRL, default_USER_CTRL);
  delay(20);
  spi_writeIMUByte(address_PWR_MGMT_1, default_PWR_MGMT_1);
  delay(20);
  spi_writeIMUByte(address_PWR_MGMT_2, default_PWR_MGMT_2);
  delay(20);

  Serial.print("who am I: ");
  Serial.println(spi_readIMUByte(address_WHO_AM_I));

  Serial.print("user control: ");
  Serial.println(spi_readIMUByte(address_USER_CTRL));

  Serial.print("Power Management 1: ");
  Serial.println(spi_readIMUByte(address_PWR_MGMT_1));

  Serial.print("Power Management 2: ");
  Serial.println(spi_readIMUByte(address_PWR_MGMT_2));
    
}

void imu_test(void) {
  delay(100);
  for(int i = 45; i<51; i++) {
    Serial.print(spi_readIMUByte(i));        
    Serial.print(" ");  
  }
  Serial.println("");
}


