#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "key.h"
#include "gpu.h"
#include "mouse.h"
#include "vgafon.h"

extern struct bitmap_font vgafon;

// Text message system
text_message_t text_messages[MAX_TEXT_MESSAGES] = {0};

float frequency = 0.005;
float time_offset = 1;
int red = 0, green = 0, blue = 0;
int currX = 0, currY = 0;
int mousesize = 20;

// ✅ Global framebuffer pointer
static int (*global_buf)[1280][3] = NULL;

void rainbowPixel() {
    if (!global_buf) return; 
    red   = (int)(127 * sin(frequency * currX + time_offset) + 128);
    green = (int)(127 * sin(frequency * currX + time_offset + 2 * M_PI / 3) + 128);
    blue  = (int)(127 * sin(frequency * currX + time_offset + 4 * M_PI / 3) + 128);
}

void putpixel(int colorR, int colorG, int colorB, int x, int y) {
    if (!global_buf) return; 
    global_buf[y][x][0] = colorR;
    global_buf[y][x][1] = colorG;
    global_buf[y][x][2] = colorB;
}

int find_glyph_index(const struct bitmap_font *f, unsigned int unicode) {
    for (int i = 0; i < f->Chars; i++) {
        if (f->Index[i] == unicode) return i;
    }
    return -1;
}

void drawchar(unsigned int c, int x, int y, int fgR, int fgG, int fgB) {
    if (!global_buf) return; 
    const struct bitmap_font *f = &vgafon;
    int glyph_index = find_glyph_index(f, c);
    if (glyph_index < 0) return;

    const unsigned char *glyph = f->Bitmap + glyph_index * f->Height;
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    for (int cy = 0; cy < f->Height; cy++) {
        for (int cx = 0; cx < f->Widths[glyph_index]; cx++) {
            int byte = glyph[cy];
            if (byte & mask[7 - cx]) {
                putpixel(fgR, fgG, fgB, x + cx, y + cy);
            }
        }
    }
}

void drawstring(char *t, int x, int y, int fgR, int fgG, int fgB) {
    if (!global_buf) return; 
    int xpos = x;
    for (int i = 0; t[i] != '\0'; i++) {
        drawchar(t[i], xpos, y, fgR, fgG, fgB);
        xpos += 10;
    }
}

void add_text_message(const char* text, int x, int y, int r, int g, int b) {
    for (int i = 0; i < MAX_TEXT_MESSAGES; i++) {
        if (!text_messages[i].active) {
            strncpy(text_messages[i].text, text, sizeof(text_messages[i].text) - 1);
            text_messages[i].text[sizeof(text_messages[i].text) - 1] = '\0';
            text_messages[i].x = x;
            text_messages[i].y = y;
            text_messages[i].r = r;
            text_messages[i].g = g;
            text_messages[i].b = b;
            text_messages[i].active = 1;
            break;
        }
    }
}

void clear_text_messages() {
    for (int i = 0; i < MAX_TEXT_MESSAGES; i++) {
        text_messages[i].active = 0;
    }
}

void drawBg(int r, int g, int b) {
    if (!global_buf) return; 
    for (int h = 0; h < 800; h++) {
        for (int w = 0; w < 1280; w++) {
            global_buf[h][w][0] = r;
            global_buf[h][w][1] = g;
            global_buf[h][w][2] = b;
        }
    }
}

void drawMouse() {
    if (!global_buf) return; 
    for (int h = 0; h < 800; h++) {
        for (int w = 0; w < 1280; w++) {
            if (w < mouse_x + mousesize && w > mouse_x && h < mouse_y + mousesize && h > mouse_y) {
                global_buf[h][w][0] = 255;
                global_buf[h][w][1] = 0;
                global_buf[h][w][2] = 0;
            }
        }
    }
}

void draw(int buf[800][1280][3], volatile int *running) {
    global_buf = buf; // 💥 Save globally

    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error opening framebuffer");
        exit(EXIT_FAILURE);
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1 ||
        ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading screen info");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    long int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == (void *)-1) {
        perror("Error mmap framebuffer");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    printf("Drawing... Press middle mouse to green, right to blue, left to red.\n");

    while (*running) {
        if (bLeft)       drawBg(120, 0, 0);
        else if (bMiddle) drawBg(0, 120, 0);
        else if (bRight)  drawBg(0, 0, 120);
        else              drawBg(0, 128, 127);

        // Draw all text messages from main.c
        for (int i = 0; i < MAX_TEXT_MESSAGES; i++) {
            if (text_messages[i].active) {
                drawstring(text_messages[i].text, text_messages[i].x, text_messages[i].y, 
                          text_messages[i].r, text_messages[i].g, text_messages[i].b);
            }
        }
        
        drawMouse();

        for (int y = 0; y < vinfo.yres; y++) {
            for (int x = 0; x < vinfo.xres; x++) {
                long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                                (y + vinfo.yoffset) * finfo.line_length;
                currX = x;
                currY = y;

                if (vinfo.bits_per_pixel == 32) {
                    fbp[location]     = global_buf[y][x][2]; // Blue
                    fbp[location + 1] = global_buf[y][x][1]; // Green
                    fbp[location + 2] = global_buf[y][x][0]; // Red
                    fbp[location + 3] = 0;
                }
            }
        }

        usleep(30000);
    }

    munmap(fbp, screensize);
    close(fbfd);
}
