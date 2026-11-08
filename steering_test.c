#include <stdio.h>
#include <unistd.h>
#include "pca9685.h"
#include "steering.h"
#include "esc.h" 

int main(void) {

int file = pca9685_init();

if (file < 0) {

    printf("PCA9685 initialization failed\n");
    return 1;
}

if (esc_neutral(file) < 0) {

    printf("Failed to set ESC Neutral\n");
    close(file);
    return 1;
}

printf("ESC Neutral active. Turn ESC on, then press Enter.\n");
getchar();

steering_center(file);
sleep(1);
steering_left(file);
sleep(1);
steering_right(file);
sleep(1);

steering_center(file);

close(file);
return 0;
}
