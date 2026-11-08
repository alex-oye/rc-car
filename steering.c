#include "pca9685.h"
#include "steering.h"

#define STEERING_LEFT_COUNT 270
#define STEERING_RIGHT_COUNT 345
#define STEERING_CENTER_COUNT 335

int steering_left(int file) {
    return pca9685_set_channel0_pwm(file, 0, STEERING_LEFT_COUNT);
}

int steering_right(int file) {
    return pca9685_set_channel0_pwm(file, 0, STEERING_RIGHT_COUNT);
}

int steering_center(int file) {
    return pca9685_set_channel0_pwm(file, 0, STEERING_CENTER_COUNT);
}
