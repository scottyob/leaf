#ifndef IMU_h
#define IMU_h

#include <Arduino.h>
#include "Leaf_SPI.h"

#define SPI_IMU_SS 21

void imu_init(void);
void imu_test(void);

#endif
