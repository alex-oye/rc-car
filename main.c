#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "steering.h"
#include "pca9685.h"
#include "esc.h"

int main(void) {

    int file = pca9685_init();

    if (file < 0) {

        printf("PCA9685 Initialization failed\n");
	return 1;
    }

    if (esc_neutral(file) < 0) {

        printf("Failed to set ESC neutral\n");
	close(file);
        return 1;
    }

    if (steering_center(file) < 0) {

        printf("Failed to center steering\n");
        close(file);
        return -1;
    }

    struct termios oldTerm;
    struct termios newTerm;

    if (tcgetattr(STDIN_FILENO, &oldTerm) < 0) {
	perror("Failed to read terminal settings");
        close(file);
	return 1;
    }

    newTerm = oldTerm;

    newTerm.c_lflag &= ~(ICANON | ECHO);
    newTerm.c_cc[VMIN] = 1;
    newTerm.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) < 0) {

        perror("Failed to change terminal settings");
	close(file);
        return 1;
    }
    
    int driveState = 0;

    printf("Arrow keys = drive | Space bar = stop/center | q = quit\n");

    int running = 1;

    while (running) {

        unsigned char key;

	if (read(STDIN_FILENO, &key, 1) != 1) {

            continue;
	}
        
	if (key == 'q') {

            running = 0;
	}

	else if (key == ' ') {

            esc_neutral(file);
	    steering_center(file);
            driveState = 0;
	}

	else if (key == 27) {

            unsigned char sequence[2];

	if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
            continue;
	}

	if (read(STDIN_FILENO, &sequence[1], 1) != 1) {

            continue;
	}

	if (sequence[0] == '[') {

	    if (sequence[1] == 'A') {
                if (driveState != 1) {
		    esc_startup(file);
		    usleep(200000);
		}
		esc_forward(file);
		driveState = 1;
	    }
	    else if (sequence[1] == 'B') {
		esc_reverse(file);
		driveState = -1;
	    }
	    else if (sequence[1] == 'C') {
                steering_right(file);
	    }
	    else if (sequence[1] == 'D') {
                steering_left(file);
            }
	}
    }
}

esc_neutral(file);
steering_center(file);

tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);

close(file);

return 0;

}
