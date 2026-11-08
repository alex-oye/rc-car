#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>

#include "steering.h"
#include "pca9685.h"
#include "esc.h"

volatile sig_atomic_t stopRequested = 0;

/* Called if Ctrl+C, SSH disconnect, or termination occurs */
void handle_signal(int signal) {
    (void)signal;
    stopRequested = 1;
}

int main(void) {

    int file = pca9685_init();

    if (file < 0) {
        printf("PCA9685 initialization failed\n");
        return 1;
    }

    /* Start the car in a safe state */
    if (esc_neutral(file) < 0) {
        printf("Failed to set ESC neutral\n");
        close(file);
        return 1;
    }

    if (steering_center(file) < 0) {
        printf("Failed to center steering\n");
        close(file);
        return 1;
    }

    /*
     * Save normal terminal settings so we can restore them
     * when the program exits.
     */
    struct termios oldTerm;
    struct termios newTerm;

    if (tcgetattr(STDIN_FILENO, &oldTerm) < 0) {
        perror("Failed to read terminal settings");
        close(file);
        return 1;
    }

    newTerm = oldTerm;

    /* Read keys immediately without echoing them */
    newTerm.c_lflag &= ~(ICANON | ECHO);
    newTerm.c_cc[VMIN] = 1;
    newTerm.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) < 0) {
        perror("Failed to change terminal settings");
        close(file);
        return 1;
    }

    /*
     * Let signals request a clean shutdown instead of
     * immediately killing the program.
     */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);

    /*
     * driveState:
     *  0 = neutral
     *  1 = forward
     * -1 = reverse
     */
    int driveState = 0;
    int running = 1;

    printf("Arrow keys = drive/steer\n");
    printf("Space = stop + center\n");
    printf("q = quit\n");

    while (running && !stopRequested) {

        unsigned char key;

        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        /*
         * Failsafe:
         * If no keyboard input arrives for 0.6 seconds,
         * automatically put the ESC into neutral.
         */
        timeout.tv_sec = 0;
        timeout.tv_usec = 600000;

        int ready = select(
            STDIN_FILENO + 1,
            &readfds,
            NULL,
            NULL,
            &timeout
        );

        if (ready < 0) {

            if (errno == EINTR) {
                break;
            }

            perror("select");
            break;
        }

        /*
         * Timeout = arrow key was released / input stopped.
         */
        if (ready == 0) {

            if (driveState != 0) {
                esc_neutral(file);
                driveState = 0;
            }

            continue;
        }

        ssize_t bytesRead = read(STDIN_FILENO, &key, 1);

        /*
         * SSH connection/stdin closed.
         */
        if (bytesRead == 0) {
            break;
        }

        if (bytesRead < 0) {

            if (errno == EINTR) {
                break;
            }

            continue;
        }

        /*
         * Q = safely quit
         */
        if (key == 'q') {
            running = 0;
        }

        /*
         * Space = immediate stop + center steering
         */
        else if (key == ' ') {

            esc_neutral(file);
            steering_center(file);

            driveState = 0;
        }

        /*
         * Arrow keys begin with ESC (27).
         *
         * Up    = ESC [ A
         * Down  = ESC [ B
         * Right = ESC [ C
         * Left  = ESC [ D
         */
        else if (key == 27) {

            unsigned char sequence[2];

            if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
                continue;
            }

            if (read(STDIN_FILENO, &sequence[1], 1) != 1) {
                continue;
            }

            if (sequence[0] == '[') {

                /*
                 * UP ARROW = FORWARD
                 */
                if (sequence[1] == 'A') {

                    /*
                     * If reversing, go neutral first.
                     * This prevents an instant reverse-to-forward
                     * direction change.
                     */
                    if (driveState == -1) {

                        esc_neutral(file);
                        driveState = 0;

                        usleep(400000);
                    }

                    /*
                     * Only use the startup kick when starting
                     * from neutral.
                     */
                    if (driveState != 1) {

                        esc_startup(file);
                        usleep(500000);
                    }

                    esc_forward(file);
                    driveState = 1;
                }

                /*
                 * DOWN ARROW = REVERSE
                 */
                else if (sequence[1] == 'B') {

                    /*
                     * If moving forward, stop first and give
                     * the ESC/motor half a second before reversing.
                     */
                    if (driveState == 1) {

                        esc_neutral(file);
                        driveState = 0;

                        usleep(500000);
                    }

                    esc_reverse(file);
                    driveState = -1;
                }

                /*
                 * RIGHT ARROW
                 */
                else if (sequence[1] == 'C') {

                    steering_right(file);
                }

                /*
                 * LEFT ARROW
                 */
                else if (sequence[1] == 'D') {

                    steering_left(file);
                }
            }
        }
    }

    /*
     * SAFE SHUTDOWN
     *
     * Any normal exit, q, Ctrl+C, SSH termination, etc.
     * reaches this section.
     */
    esc_neutral(file);
    steering_center(file);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);

    close(file);

    printf("\nRC car stopped safely\n");

    return 0;
}
