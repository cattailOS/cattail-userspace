#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include "mouse.h"  

#define MOUSEFILE "/dev/input/event1"  // Change to proper event device
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800

int mouse_x = SCREEN_WIDTH / 2;
int mouse_y = SCREEN_HEIGHT / 2;
int fixMouse_x = 0;
int xOffset = SCREEN_WIDTH / 10;
int fixMouse_y = 0;
unsigned char button, bLeft, bMiddle, bRight;

void listen_mouseEv(volatile int *running)
{
    int fd;
    struct input_event ie;

    if((fd = open(MOUSEFILE, O_RDONLY | O_NONBLOCK)) == -1)
    {
        printf("NonBlocking %s open ERROR\n", MOUSEFILE);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("NonBlocking %s open OK\n", MOUSEFILE);
    }

    while (*running) {
        ssize_t bytes = read(fd, &ie, sizeof(struct input_event));
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            perror("Error reading input event");
            close(fd);
            exit(EXIT_FAILURE);
        }
        
        if (bytes == 0) {
            usleep(1000);
            continue;
        }

        if ((size_t)bytes == sizeof(struct input_event)) {
            // Handle mouse movement events
            if (ie.type == EV_REL) {
                if (ie.code == REL_X) {
                    int new_x = mouse_x + ie.value;
                    if (new_x < 20) new_x = 20;
                    if (new_x > SCREEN_WIDTH - 20) new_x = SCREEN_WIDTH - 20;
                    mouse_x = new_x;
                }
                else if (ie.code == REL_Y) {
                    int new_y = mouse_y + ie.value;
                    if (new_y < 20) new_y = 20;
                    if (new_y > SCREEN_HEIGHT - 20) new_y = SCREEN_HEIGHT - 20;
                    mouse_y = new_y;
                }
            }
            // Handle button events
            else if (ie.type == EV_KEY) {
                if (ie.code == BTN_LEFT) {
                    bLeft = ie.value;
                }
                else if (ie.code == BTN_RIGHT) {
                    bRight = ie.value;
                }
                else if (ie.code == BTN_MIDDLE) {
                    bMiddle = ie.value;
                }
            }
            
            fixMouse_x = mouse_x + 20 - xOffset;
        }
    }

    close(fd);
}