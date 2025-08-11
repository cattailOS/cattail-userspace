#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include "key.h"
#include "wm.h"
#include "gpu.h"
#include "mouse.h"
#include "vgafon.h"

extern struct bitmap_font vgafon;
int vt_fd = -1;  // Use -1 instead of NULL for invalid file descriptor
// Text message system
text_message_t text_messages[MAX_TEXT_MESSAGES] = {0};

// Mutex for framebuffer synchronization
pthread_mutex_t framebuffer_mutex = PTHREAD_MUTEX_INITIALIZER;

float frequency = 0.005;
float time_offset = 1;
int red = 0, green = 0, blue = 0;
int currX = 0, currY = 0;
int mousesize = 20;

// ✅ Global framebuffer pointer
static int (*global_buf)[1280][3] = NULL;
static int (*global_curbuf)[1280][3] = NULL;
static int (*global_bgbuf)[1280][3] = NULL;
void rainbowPixel()
{
    if (!global_buf)
        return;
    red = (int)(127 * sin(frequency * currX + time_offset) + 128);
    green = (int)(127 * sin(frequency * currX + time_offset + 2 * M_PI / 3) + 128);
    blue = (int)(127 * sin(frequency * currX + time_offset + 4 * M_PI / 3) + 128);
}

void putpixel(int colorR, int colorG, int colorB, int x, int y)
{
    if (!global_buf)
        return;
    global_buf[y][x][0] = colorR;
    global_buf[y][x][1] = colorG;
    global_buf[y][x][2] = colorB;
}

int find_glyph_index(const struct bitmap_font *f, unsigned int unicode)
{
    for (int i = 0; i < f->Chars; i++)
    {
        if (f->Index[i] == unicode)
            return i;
    }
    return -1;
}

void drawchar(unsigned int c, int x, int y, int fgR, int fgG, int fgB)
{
    if (!global_buf)
        return;
    const struct bitmap_font *f = &vgafon;
    int glyph_index = find_glyph_index(f, c);
    if (glyph_index < 0)
        return;

    const unsigned char *glyph = f->Bitmap + glyph_index * f->Height;
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    for (int cy = 0; cy < f->Height; cy++)
    {
        for (int cx = 0; cx < f->Widths[glyph_index]; cx++)
        {
            int byte = glyph[cy];
            if (byte & mask[7 - cx])
            {
                putpixel(fgR, fgG, fgB, x + cx, y + cy);
            }
        }
    }
}

void drawstring(char *t, int x, int y, int fgR, int fgG, int fgB)
{
    if (!global_buf)
        return;

    int xpos = x;
    for (int i = 0; t[i] != '\0'; i++)
    {
        drawchar(t[i], xpos, y, fgR, fgG, fgB);
        xpos += 10;
    }
}

// Thread-safe version of drawstring
void drawstring_safe(char *t, int x, int y, int fgR, int fgG, int fgB)
{
    pthread_mutex_lock(&framebuffer_mutex);
    drawstring(t, x, y, fgR, fgG, fgB);
    pthread_mutex_unlock(&framebuffer_mutex);
}

void add_text_message(const char *text, int x, int y, int r, int g, int b)
{
    for (int i = 0; i < MAX_TEXT_MESSAGES; i++)
    {
        if (!text_messages[i].active)
        {
            snprintf(text_messages[i].text, sizeof(text_messages[i].text), "%s", text);
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

void clear_text_messages()
{
    for (int i = 0; i < MAX_TEXT_MESSAGES; i++)
    {
        text_messages[i].active = 0;
    }
}

void drawBg(int r, int g, int b)
{
    if (!global_bgbuf)
        return;
    for (int h = 0; h < 800; h++)
    {
        for (int w = 0; w < 1280; w++)
        {
            global_bgbuf[h][w][0] = r;
            global_bgbuf[h][w][1] = g;
            global_bgbuf[h][w][2] = b;
        }
    }
}

void clearForeground()
{
    if (!global_buf)
        return;
    for (int h = 0; h < 800; h++)
    {
        for (int w = 0; w < 1280; w++)
        {
            global_buf[h][w][0] = 256;
            global_buf[h][w][1] = 256;
            global_buf[h][w][2] = 256;
        }
    }
}
void smartClearFgOld()
{
    int fgMask[800][1280] = {0};
    for (size_t i = 0; i < window_count; i++)
    {
        for (int h = 0; h < 800; h++)
        {
            for (int w = 0; w < 1280; w++)
            {
                if(w < windows[i].x && w > windows[i].x + windows[i].width && h < windows[i].y && h > windows[i].y + windows[i].height){
                    fgMask[h][w] = 0;
                }
                else{
                    fgMask[h][w] = 1;
                }
            }
        }
    }

    if (!global_buf)
        return;
    for (int h = 0; h < 800; h++)
    {
        for (int w = 0; w < 1280; w++)
        {
            if(fgMask[h][w] == 0){
                global_buf[h][w][0] = 256;
                global_buf[h][w][1] = 256;
                global_buf[h][w][2] = 256;
            }
        }   
    }
}

void smartClearFg()
{
    // Initialize the entire mask to 0 (no window)
    int fgMask[800][1280] = {0};

    // For each window, mark the pixels inside it as 1
    for (size_t i = 0; i < window_count; i++)
    {
        // Iterate only over the pixels within the current window's bounds
        for (int h = windows[i].y; h < windows[i].y + windows[i].height; h++)
        {
            for (int w = windows[i].x; w < windows[i].x + windows[i].width; w++)
            {
                // Boundary check to ensure we don't write out of fgMask's bounds
                if (h >= 0 && h < 800 && w >= 0 && w < 1280)
                {
                    fgMask[h][w] = 1;
                }
            }
        }
    }

    if (!global_buf)
        return;

    // Iterate through the screen and clear pixels where the mask is 0
    for (int h = 0; h < 800; h++)
    {
        for (int w = 0; w < 1280; w++)
        {
            if(fgMask[h][w] == 0){
                // A value of 255 is standard for the highest value in an 8-bit color channel.
                // If your system requires 256, you can leave it.
                global_buf[h][w][0] = 256;
                global_buf[h][w][1] = 256;
                global_buf[h][w][2] = 256;
            }
        }
    }
}
void clearcurbuf()
{
    if (!global_curbuf)
        return;
    for (int h = 0; h < 800; h++)
    {
        for (int w = 0; w < 1280; w++)
        {
            global_curbuf[h][w][0] = 256;
            global_curbuf[h][w][1] = 256;
            global_curbuf[h][w][2] = 256;
        }
    }
}
void drawMouse()
{
    char mouse_pointer[13][21] = {
        "X",
        "XXX",
        "XXXXX",
        "XXXXXXX",
        "XXXXXXXXXX",
        "XXXXXXXXXXXX",
        "XXXXXXXXXXXXXX",
        "XXXXXXXXXXXXXXXX",
        "XXXXXXXXXX",
        "XXX_XXXXXX",
        "X____XXXXXX",
        "______XXXXXX",
        "_______XXXXX",
    };

    if (!global_curbuf)
        return;

    for (int y = 0; y < 13; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            int realy = y;
            if (mouse_pointer[realy][x] == 'X')
            {
                int draw_x = mouse_x + x;
                int draw_y = mouse_y + y;

                // Bounds check
                if (draw_x >= 0 && draw_x < 1280 && draw_y >= 0 && draw_y < 800)
                {
                    global_curbuf[draw_y][draw_x][0] = 255; // Red
                    global_curbuf[draw_y][draw_x][1] = 255;
                    global_curbuf[draw_y][draw_x][2] = 255;
                }
            }
        }
    }
}

void draw(int buf[800][1280][3], int bgbuf[800][1280][3], volatile int *running)
{
    int target_vt = 2; // Set the target virtual terminal (e.g., tty2)
    int vt_fd = open("/dev/tty2", O_RDWR);
    ioctl(vt_fd, VT_ACTIVATE, target_vt);
    ioctl(vt_fd, VT_WAITACTIVE, target_vt);
    ioctl(vt_fd, KDSETMODE, KD_GRAPHICS);
    global_buf = buf;     // 💥 Save globally
    global_bgbuf = bgbuf; // 💥 Save globally

    // Allocate memory for the current frame buffer
    static int curbuf[800][1280][3];
    global_curbuf = curbuf;

    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1)
    {
        perror("Error opening framebuffer");
        exit(EXIT_FAILURE);
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1 ||
        ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading screen info");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    long int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == (void *)-1)
    {
        perror("Error mmap framebuffer");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    printf("Drawing... Press middle mouse to green, right to blue, left to red.\n");
    drawBg(0, 128, 127);
    clearForeground();
    clearcurbuf();

    while (*running)
    {
        // Clear the foreground buffer to transparent at the start of each frame
        // clearForeground();
        drawMouse();
        // if (bLeft)
        //     drawBg(120, 0, 0);
        // else if (bMiddle)
        //     drawBg(0, 120, 0);
        // else if (bRight)
        //     drawBg(0, 0, 120);
        // else
        drawBg(0, 128, 127);

        // Draw all text messages from main.c
        for (int i = 0; i < MAX_TEXT_MESSAGES; i++)
        {
            if (text_messages[i].active)
            {
                drawstring(text_messages[i].text, text_messages[i].x, text_messages[i].y,
                           text_messages[i].r, text_messages[i].g, text_messages[i].b);
            }
        }

        for (int y = 0; y < vinfo.yres; y++)
        {
            for (int x = 0; x < vinfo.xres; x++)
            {
                long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                                (y + vinfo.yoffset) * finfo.line_length;
                currX = x;
                currY = y;

                if (vinfo.bits_per_pixel == 32)
                {
                    fbp[location] = global_bgbuf[y][x][2];     // Blue
                    fbp[location + 1] = global_bgbuf[y][x][1]; // Green
                    fbp[location + 2] = global_bgbuf[y][x][0]; // Red
                    fbp[location + 3] = 0;
                    // Check if pixel is NOT transparent (all channels must be 256 for transparency)
                    if (global_buf[y][x][2] != 256 && global_buf[y][x][1] != 256 && global_buf[y][x][0] != 256)
                    {
                        fbp[location] = global_buf[y][x][2];     // Blue
                        fbp[location + 1] = global_buf[y][x][1]; // Green
                        fbp[location + 2] = global_buf[y][x][0]; // Red
                    }
                    if (global_curbuf[y][x][2] != 256 && global_curbuf[y][x][1] != 256 && global_curbuf[y][x][0] != 256)
                    {
                        fbp[location] = global_curbuf[y][x][2];     // Blue
                        fbp[location + 1] = global_curbuf[y][x][1]; // Green
                        fbp[location + 2] = global_curbuf[y][x][0]; // Red
                    }
                    fbp[location + 3] = 0;
                }
            }
        }
        clearcurbuf();
        smartClearFg();
        // usleep(30000);
    }

    munmap(fbp, screensize);
    close(fbfd);
}
