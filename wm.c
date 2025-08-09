#include "mouse.h"
#include "gpu.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int last_win_id = -1;
int last_win_xoffset = -1;
int last_win_yoffset = -1;

typedef struct
{
    int x, y;
    int width, height;
    char title[64];

    int owner_id;
    int win_id;

    int ***buffer; // [height][width][3]
} Window;

Window *windows = NULL;
size_t window_count = 0;

void add_window(Window new_window)
{
    windows = realloc(windows, sizeof(Window) * (window_count + 1));
    windows[window_count++] = new_window;
}
int ***alloc_rgb_buffer(int height, int width)
{
    int ***buf = malloc(height * sizeof(int **));
    for (int y = 0; y < height; y++)
    {
        buf[y] = malloc(width * sizeof(int *));
        for (int x = 0; x < width; x++)
        {
            buf[y][x] = malloc(3 * sizeof(int)); // RGB
        }
    }
    return buf;
}
void free_rgb_buffer(int ***buf, int height, int width)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            free(buf[y][x]);
        }
        free(buf[y]);
    }
    free(buf);
}
Window create_window(int x, int y, int w, int h, const char *title, int owner_id, int app_id)
{
    Window win;
    win.x = x;
    win.y = y;
    win.width = w;
    win.height = h;
    strncpy(win.title, title, 63);
    win.title[63] = '\0';
    win.owner_id = owner_id;
    win.win_id = app_id;
    win.buffer = alloc_rgb_buffer(h, w);
    return win;
}
void destroy_windows(Window *windows, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        free_rgb_buffer(windows[i].buffer, windows[i].height, windows[i].width);
    }
    free(windows);
}

// Initialise an empty buf
static int (*buf)[800][1280][3] = NULL;
volatile int wmActive = 0;

// Pull a pointer to our actual buf in from the caller
void wm_init(int bufptr[800][1280][3])
{
    // make int buf be a pointer to whatever bufptr is pointing
    buf = (int (*)[800][1280][3])bufptr;
    add_window(create_window(200, 200, 100, 100, "Window!", 69, 420));
}

void wm_draw(volatile int *running)
{
    while (*running)
    {
        for (size_t i = 0; i < window_count; i++)
        {
            // Check if mouse is over window and left button pressed
            if (mouse_x >= windows[i].x && mouse_x < windows[i].x + windows[i].width &&
                mouse_y >= windows[i].y && mouse_y < windows[i].y + 25 && bLeft)
            {
                if (last_win_id != i)
                {
                    // Start dragging - calculate offset from window top-left
                    last_win_id = i;
                    last_win_xoffset = mouse_x - windows[i].x;
                    last_win_yoffset = mouse_y - windows[i].y;
                }
            }
            else if (last_win_id == i && !bLeft)
            {
                // Stop dragging
                last_win_id = -1;
            }

            // Update window position if dragging
            if (last_win_id == i && bLeft)
            {
                // Calculate new position
                int new_x = mouse_x - last_win_xoffset;
                int new_y = mouse_y - last_win_yoffset;

                // Clamp to screen bounds
                if (new_x < 0)
                    new_x = 0;
                if (new_y < 0)
                    new_y = 0;
                if (new_x + windows[i].width > 1280)
                    new_x = 1280 - windows[i].width;
                if (new_y + windows[i].height > 800)
                    new_y = 800 - windows[i].height;

                windows[i].x = new_x;
                windows[i].y = new_y;
            }

            // Draw window - with bounds checking
            for (int h = windows[i].y; h < windows[i].y + windows[i].height && h < 800; h++)
            {
                if (h < 0)
                    continue; // Skip negative rows
                for (int w = windows[i].x; w < windows[i].x + windows[i].width && w < 1280; w++)
                {
                    if (w < 0)
                        continue; // Skip negative columns
                    // Check if mouse is over this window
                    if (mouse_x >= windows[i].x && mouse_x < windows[i].x + windows[i].width &&
                        mouse_y >= windows[i].y && mouse_y < windows[i].y + 25)
                    {
                        // Highlighted color
                        (*buf)[h][w][0] = 50;
                        (*buf)[h][w][1] = 255;
                        (*buf)[h][w][2] = 50;
                    }
                    else
                    {
                        // Normal color
                        (*buf)[h][w][0] = 50;
                        (*buf)[h][w][1] = 50;
                        (*buf)[h][w][2] = 255;
                    }
                    if (w >= windows[i].x && w < windows[i].x + windows[i].width &&
                        h >= windows[i].y && h < windows[i].y + 25)
                    {
                        (*buf)[h][w][0] = 100;
                        (*buf)[h][w][1] = 100;
                        (*buf)[h][w][2] = 255;
                    }
                }
                drawstring(windows[i].title, windows[i].x + 5, windows[i].y + 5, 255, 255, 255);
            }
            usleep(30000);
        }
    }
}