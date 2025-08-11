#include <unistd.h>     // for read(), close()
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include "mouse.h"  

#define MOUSEFILE "/dev/input/mice\0"
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800
int mouse_x = SCREEN_WIDTH / 2;
int mouse_y = SCREEN_HEIGHT / 2;
int fixMouse_x = 0;
int xOffset = SCREEN_WIDTH / 10;
int fixMouse_y = 0;
unsigned char button,bLeft,bMiddle,bRight;
void listen_mouseEv(volatile int *running)
{
    int fd;
    struct input_event ie;
    unsigned char *ptr = (unsigned char*)&ie;
    //
    char x,y;                                                            // the relX , relY datas
    // int mouse_x,mouse_y;

    if((fd = open(MOUSEFILE, O_RDONLY | O_NONBLOCK )) == -1)
    {
        printf("NonBlocking %s open ERROR\n",MOUSEFILE);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("NonBlocking %s open OK\n",MOUSEFILE);
    }


    while (*running) {
        // Read an input event
        ssize_t bytes = read(fd, &ie, sizeof(struct input_event));
        if (bytes < 0) {
            perror("Error reading input event");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Ensure we read a full input_event structure and prevent buffer overflow
        if ((size_t)bytes == sizeof(struct input_event)) {
            button = ptr[0];
            bLeft = button & 0x1;
            bMiddle = (button & 0x4) > 0;
            bRight = (button & 0x2) > 0;
            x = (char)ptr[1];
            y = (char)ptr[2];
            mouse_x += x;
            mouse_y -= y;
            if (mouse_x + 20 < 1) mouse_x = 20;
            if (mouse_x + 20 >SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 20;
            if (mouse_y + 21 < 0) mouse_y = 20;
            if (mouse_y + 21 >SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 20;
            fixMouse_x = mouse_x + 20 - xOffset;
            fflush(stdout);
        }
    }

    close(fd);
}