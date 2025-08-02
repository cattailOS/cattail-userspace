#ifndef MOUSE_H
#define MOUSE_H

void listen_mouseEv(volatile int *running);
extern int mouse_x          ;
extern int mouse_y;
extern int fixMouse_x;
extern unsigned char bLeft;
extern unsigned char bMiddle;
extern unsigned char bRight;


#endif
