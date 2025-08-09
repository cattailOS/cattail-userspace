#ifndef GPU_H
#define GPU_H

#include <pthread.h>

// Text message structure for drawing from main.c
typedef struct {
    char text[256];
    int x, y;
    int r, g, b;
    int active;
} text_message_t;

#define MAX_TEXT_MESSAGES 10
extern text_message_t text_messages[MAX_TEXT_MESSAGES];
extern pthread_mutex_t framebuffer_mutex;

void draw(int buf[800][1280][3], int bgbuf[800][1280][3], volatile int *running);
void putpixel(int colorR, int colorG, int colorB, int x, int y);
void drawchar(unsigned int c, int x, int y, int fgR, int fgG, int fgB);
void drawstring(char* t, int x, int y, int fgR, int fgG, int fgB);
void drawstring_safe(char* t, int x, int y, int fgR, int fgG, int fgB);
void add_text_message(const char* text, int x, int y, int r, int g, int b);
void clear_text_messages();
void init_framebuffer_mutex();
void clearForeground();
#endif
