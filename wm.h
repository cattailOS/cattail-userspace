#ifndef WM_H
#define WM_H

void wm_init(int bufptr[800][1280][3]);
void wm_draw(volatile int *running);
// void wm_stopRendering(void);
// void wm_restartRendering(void);

#endif // WM_H