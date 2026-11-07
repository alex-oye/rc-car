#ifndef PCA9685_H
#define PCA9685_H

#include <stdint.h>

int pca9685_init(void);
int pca9685_set_channel0_pwm(int file, uint16_t onCount, uint16_t offCount);
int pca9685_set_channel1_pwm(int file, uint16_t onCount, uint16_t offCount);

#endif
