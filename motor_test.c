#include <stdio.h> 
#include <unistd.h>
#include "pca9685.h"
#include "esc.h"

int main(void) {
 
int file = pca9685_init();

if (file < 0) {
    printf("error");
    return 1;
}

if (esc_neutral(file) < 0) {

    printf("error");
    close(file);
    return 1;
}

printf("Neutral active, power on esc and press enter\n");
getchar();

esc_startup(file);
usleep(200000);
esc_forward(file);
sleep(2);
esc_neutral(file);
close(file);
return 0;
}
