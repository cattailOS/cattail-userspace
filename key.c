#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "key.h"
#include "mouse.h"

void listen_for_keys(const char *device, volatile int *running) {
    int fd;
    struct input_event ev;

    // Open the keyboard device file
    fd = open(device, O_RDONLY);
    if (fd == -1) {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }

    printf("Listening for key presses on %s... (Press key 35 to exit)\n", device);

    // Event loop
    while (*running) {
        // Read an input event
        ssize_t bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes == (ssize_t)-1) {
            perror("Error reading input event");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Ensure we read a full input_event structure
        if (bytes != sizeof(struct input_event)) {
            fprintf(stderr, "Incomplete input event read\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Check if the event is a key event
        if (ev.type == EV_KEY && ev.value == 1) { // Key press
            printf("Key pressed: %d\n", ev.code);

            // Exit the loop if key 35 is pressed
            if (ev.code == 35) {
                printf("Key 35 pressed. Exiting...\n");
                *running = 0; // Signal the other thread to stop
		        system("bash -c clear > /dev/tty2");
                break;
            }
        }
    }

    close(fd);
}

