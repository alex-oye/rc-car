#include "pca9685.h"
#include "esc.h"

#define ESC_NEUTRAL_COUNT 333
#define ESC_STARTUP_COUNT 400
#define ESC_REVERSE_COUNT 295
#define ESC_FORWARD_COUNT 380


int esc_neutral(int file) {
    return pca9685_set_channel1_pwm(file, 0, ESC_NEUTRAL_COUNT);
}

int esc_startup(int file) {
    return pca9685_set_channel1_pwm(file, 0, ESC_STARTUP_COUNT);
}

int esc_forward(int file) {
    return pca9685_set_channel1_pwm(file, 0, ESC_FORWARD_COUNT);
}


int esc_reverse(int file) {
    return pca9685_set_channel1_pwm(file, 0, ESC_REVERSE_COUNT);
}

