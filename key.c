#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "key.h"
#include "mouse.h"
#include "gpu.h"
#include "keycode.h"

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
        if (bytes < 0) {
            perror("Error reading input event1");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Ensure we read a full input_event structure to prevent processing partial/invalid data.
        if ((size_t)bytes < sizeof(struct input_event)) {
            // If we read 0 bytes, it might be EOF. If *running is true, we might block again.
            // If we read a partial event, it's better to log and continue or exit.
            if (bytes > 0) {
                fprintf(stderr, "Incomplete input event read (%zd bytes)\n", bytes);
            }
            continue; // Skip this partial event and try to read the next one.
        }

        // Check if the event is a key event (only process initial key press, not repeats)
        if (ev.type == EV_KEY) { // Key press (not repeat)
            // printf("Key pressed: %d\n", ev.code);

            // Exit the loop if key 35 is pressed
            // if (ev.code == 35) {
            //     printf("Key 35 pressed. Exiting...\n");
            //     *running = 0; // Signal the other thread to stop
		    //     system("bash -c clear > /dev/tty2");
            //     break;
            // }
            
            // Use direct drawstring without mutex to prevent freezing
            // Only draw on initial key press (ev.value == 1), not on repeats
            char key_text[32];
            snprintf(key_text, sizeof(key_text), "Key: %d", ev.code);
            drawstring(key_text, 150, 150, 200, 100, 100);
        }
    }

    close(fd);
}

