#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include "mouse.h"
#include <time.h>  

#define MOUSEFILE "/dev/input/event1"  // Change to proper event device
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800

time_t current_timestamp;
int bLeftDownTime = -1;
int bMiddleDownTime = -1;
int bRightDownTime = -1;

int bLeftPos = -1;
int bMiddlePos = -1;
int bRightPos = -1;


int bLeftRich = -1;
int bMiddleRich = -1;
int bRightRich = -1;

// Helper macros to pack/unpack coordinates
#define PACK_COORDS(x, y) (((x) << 16) | ((y) & 0xFFFF))
#define UNPACK_X(packed) ((packed) >> 16)
#define UNPACK_Y(packed) ((packed) & 0xFFFF)
#define IS_DRAG(start_packed, curr_x, curr_y) \
    ((start_packed != -1) && \
     (((curr_x - UNPACK_X(start_packed)) * (curr_x - UNPACK_X(start_packed)) + \
       (curr_y - UNPACK_Y(start_packed)) * (curr_y - UNPACK_Y(start_packed))) > 100))

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
        if(bLeft && time(NULL) - bLeftDownTime < 1){
            bLeftRich = 1;
        }
        else if(bLeft && time(NULL) - bLeftDownTime > 1 && !IS_DRAG(bLeftPos, mouse_x, mouse_y)){
            bLeftRich = 2;
        }
        else if(bLeft && time(NULL) - bLeftDownTime > 1 && IS_DRAG(bLeftPos, mouse_x, mouse_y)){
            bLeftRich = 3;
        }
        else{
            bLeftRich = 0;
        }
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
                    bLeftPos = PACK_COORDS(mouse_x, mouse_y);
                    bLeftDownTime = time(NULL);
                    bLeft = ie.value;
                }
                else if (ie.code == BTN_RIGHT) {
                    bLeftDownTime = time(NULL);
                    bRight = ie.value;
                }
                else if (ie.code == BTN_MIDDLE) {
                    bLeftDownTime = time(NULL);
                    bMiddle = ie.value;
                }
            }
            fixMouse_x = mouse_x + 20 - xOffset;
        }
    }

    close(fd);
}