// I love gays!
#include "mouse.h"
#include "gpu.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wm.h"

int last_win_id = -1;
int last_win_xoffset = -1;
int last_win_yoffset = -1;
int last_win_state = -1;
// State: -1: none, 0: drag, 1: resize

// Remove the typedef struct since it's now in the header
// typedef struct { ... } Window;

// Make these global variables accessible from other files
Window *windows = NULL;
size_t window_count = 0;
void move_window_to_back(size_t window_index)
{
    if (window_index >= window_count || window_index == 0)
        return; // Already at front or invalid index
    
    // Save the window to move
    Window temp_window = windows[window_index];
    
    // Shift all windows from 0 to window_index-1 one position to the right
    for (size_t i = window_index; i > 0; i--)
    {
        windows[i] = windows[i - 1];
    }
    
    // Place the saved window at index 0
    windows[0] = temp_window;
}

void move_window_to_front(size_t window_index)
{
    if (window_index >= window_count || window_index == window_count - 1)
        return; // Already at back or invalid index
    
    // Save the window to move
    Window temp_window = windows[window_index];
    
    // Shift all windows from window_index+1 to end one position to the left
    for (size_t i = window_index; i < window_count - 1; i++)
    {
        windows[i] = windows[i + 1];
    }
    
    // Place the saved window at the end
    windows[window_count - 1] = temp_window;
}

void move_window_to_position(size_t from_index, size_t to_index)
{
    if (from_index >= window_count || to_index >= window_count || from_index == to_index)
        return;
    
    Window temp_window = windows[from_index];
    
    if (from_index < to_index)
    {
        // Moving towards the end - shift elements left
        for (size_t i = from_index; i < to_index; i++)
        {
            windows[i] = windows[i + 1];
        }
    }
    else
    {
        // Moving towards the beginning - shift elements right
        for (size_t i = from_index; i > to_index; i--)
        {
            windows[i] = windows[i - 1];
        }
    }
    
    windows[to_index] = temp_window;
}
void add_window(Window new_window)
{
    Window *temp = realloc(windows, sizeof(Window) * (window_count + 1));
    if (temp == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for new window.\n");
        // The original 'windows' pointer is still valid, so we should free it
        // and its contents before exiting to avoid memory leaks.
        destroy_windows(windows, window_count);
        exit(EXIT_FAILURE);
    }
    windows = temp;
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
    snprintf(win.title, 63, "%s", title);
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

// Function to delete a window by its ID
int delete_window_by_id(int win_id)
{
    int window_index = find_window_index_by_id(win_id);
    if (window_index == -1) {
        return 0; // Window not found
    }
    
    // Clear any active dragging/resizing of this window
    if (last_win_id == win_id) {
        last_win_id = -1;
        last_win_state = -1;
        last_win_xoffset = -1;
        last_win_yoffset = -1;
    }
    
    // Free the window's buffer
    free_rgb_buffer(windows[window_index].buffer, 
                   windows[window_index].height, 
                   windows[window_index].width);
    
    // Shift all windows after this one to the left
    for (size_t i = window_index; i < window_count - 1; i++) {
        windows[i] = windows[i + 1];
    }
    
    // Decrease window count
    window_count--;
    
    // Reallocate to shrink the array (optional, for memory efficiency)
    if (window_count > 0) {
        Window *temp = realloc(windows, sizeof(Window) * window_count);
        if (temp != NULL) {
            windows = temp;
        }
        // If realloc fails, we still have the valid data, just using more memory
    } else {
        // No windows left
        free(windows);
        windows = NULL;
    }
    
    return 1; // Success
}

// Function to delete a window by array index
int delete_window_by_index(size_t window_index)
{
    if (window_index >= window_count) {
        return 0; // Invalid index
    }
    
    int win_id = windows[window_index].win_id;
    return delete_window_by_id(win_id);
}

// Initialise an empty buf
static int (*buf)[800][1280][3] = NULL;
volatile int wmActive = 0;
int wmap[800][1280] = {-1};
// Pull a pointer to our actual buf in from the caller
void wm_init(int bufptr[800][1280][3])
{
    // make int buf be a pointer to whatever bufptr is pointing
    buf = (int (*)[800][1280][3])bufptr;
    //                                                  ___      __
    //                                    Application ID   \/  \/  Fixed window ID to identify on layer changes
    add_window(create_window(200, 200, 100, 100, "Window!", 5, 0));
    add_window(create_window(300, 400, 100, 100, "Window2!", 4, 1));
}

// Function to find a window by its unique ID
Window* find_window_by_id(int win_id)
{
    for (size_t i = 0; i < window_count; i++) {
        if (windows[i].win_id == win_id) {
            return &windows[i];
        }
    }
    return NULL;
}

// Function to find the array index of a window by its ID
int find_window_index_by_id(int win_id)
{
    for (size_t i = 0; i < window_count; i++) {
        if (windows[i].win_id == win_id) {
            return (int)i;
        }
    }
    return -1;
}

// Modified move functions that return pointer to moved window
Window* move_window_to_front_and_get(size_t window_index)
{
    if (window_index >= window_count || window_index == window_count - 1)
        return &windows[window_index]; // Already at back or invalid index
    
    // Save the window to move
    Window temp_window = windows[window_index];
    
    // Shift all windows from window_index+1 to end one position to the left
    for (size_t i = window_index; i < window_count - 1; i++)
    {
        windows[i] = windows[i + 1];
    }
    
    // Place the saved window at the end
    windows[window_count - 1] = temp_window;
    
    // Return pointer to the moved window (now at the end)
    return &windows[window_count - 1];
}

Window* move_window_to_back_and_get(size_t window_index)
{
    if (window_index >= window_count || window_index == 0)
        return &windows[window_index]; // Already at front or invalid index
    
    // Save the window to move
    Window temp_window = windows[window_index];
    
    // Shift all windows from 0 to window_index-1 one position to the right
    for (size_t i = window_index; i > 0; i--)
    {
        windows[i] = windows[i - 1];
    }
    
    // Place the saved window at index 0
    windows[0] = temp_window;
    
    // Return pointer to the moved window (now at index 0)
    return &windows[0];
}


void wm_draw(volatile int *running)
{
    // static int wmap[800][1280];
    static int last_mouse_x = -1, last_mouse_y = -1;
    static int last_close_time = 0;
    
    while (*running)
    {
        // Only rebuild wmap if something changed
        int needs_update = (mouse_x != last_mouse_x || mouse_y != last_mouse_y || 
                           last_win_id != -1);
        
        if (needs_update) {
            memset(wmap, -1, sizeof(wmap));

            // Map window indices
            for (size_t i = 0; i < window_count; i++) {
                for (int y = windows[i].y; y < windows[i].y + windows[i].height; y++) {
                    for (int x = windows[i].x; x < windows[i].x + windows[i].width; x++) {
                        if (x >= 0 && x < 1280 && y >= 0 && y < 800)
                            wmap[y][x] = i;
                    }
                }
            }
            
            // Handle window interaction (clicking to start drag/resize)
            for (size_t pi = 0; pi < window_count; pi++)
            {
                Window *win = &windows[pi];
                int win_id = win->win_id;
                if(pi != window_count - 1){
                    if(mouse_x < 1280 && mouse_y < 800 && mouse_x >= 0 && mouse_y >= 0){
                        if(wmap[(int)fmin(fmax(0, mouse_y), 799)][(int)fmin(fmax(0, mouse_x), 1279)] == pi){

                        }
                        else {
                            continue;
                        }
                    }
                }
                if (bLeftRich == 1)
                {
                    for(int wi = 0; wi < window_count; wi++){
                    // printf("Clicked titlebar of Window %zu (titled: %s)\n", pi, windows[pi].title);
                        printf("Window %zu (titled: %s)\n", wi, windows[wi].title);

                    }
                }
                // Check if mouse is over window title bar and left button pressed
                if (mouse_x >= win->x && mouse_x < win->x + win->width - 10 &&
                    mouse_y >= win->y && mouse_y < win->y + 25 && bLeft && 
                    last_win_id == -1 && last_win_state == -1)
                {
                    // Move window to front and get new pointer
                    Window *moved_win = move_window_to_front_and_get(pi);
                    
                    // Start dragging
                    last_win_id = win_id;
                    last_win_state = 0;
                    last_win_xoffset = mouse_x - moved_win->x;
                    last_win_yoffset = mouse_y - moved_win->y;
                    break; // Important: break after moving to avoid processing moved window again
                }
                // Check resize handle
                else if (mouse_x >= win->x + win->width - 10 && mouse_x < win->x + win->width &&
                    mouse_y >= win->y + win->height - 10 && mouse_y < win->y + win->height && 
                    bLeft && last_win_id == -1 && last_win_state == -1)
                {
                    // Move window to front and get new pointer
                    Window *moved_win = move_window_to_front_and_get(pi);
                    
                    // Start resizing
                    last_win_id = win_id;
                    last_win_state = 1;
                    last_win_xoffset = mouse_x - moved_win->x;
                    last_win_yoffset = mouse_y - moved_win->y;
                    break; // Important: break after moving
                }
                else if (mouse_x >= win->x + win->width - 10 && mouse_x < win->x + win->width &&
                         mouse_y >= win->y && mouse_y <= win->y + 25 && 
                         (bLeftRich == 1 || bLeftRich == 2) && time(NULL) != last_close_time){
                    delete_window_by_id(win_id);
                    last_close_time = time(NULL);
                    printf("Closed Window %d\n", win_id);
                    clearForeground();
                    break;
                }
            }
            
            // Handle stopping drag/resize
            if (last_win_id != -1 && !bLeft)
            {
                last_win_id = -1;
                last_win_state = -1;
            }

            // Handle ongoing drag/resize operations
            if (last_win_id != -1 && bLeft)
            {
                Window *active_win = find_window_by_id(last_win_id);
                if (active_win != NULL)
                {
                    if (last_win_state == 0) // Dragging
                    {
                        // Calculate new position
                        int new_x = mouse_x - last_win_xoffset;
                        int new_y = mouse_y - last_win_yoffset;

                        // Clamp to screen bounds
                        if (new_x < 0) new_x = 0;
                        if (new_y < 0) new_y = 0;
                        if (new_x + active_win->width > 1280)
                            new_x = 1280 - active_win->width;
                        if (new_y + active_win->height > 800)
                            new_y = 800 - active_win->height;

                        active_win->x = new_x;
                        active_win->y = new_y;
                    }
                    else if (last_win_state == 1) // Resizing
                    {
                        // Calculate new size
                        int new_width = mouse_x - active_win->x;
                        int new_height = mouse_y - active_win->y;
                        
                        // Set minimum window size
                        if (new_width < 50) new_width = 50;
                        if (new_height < 25) new_height = 25;
                        
                        // Set maximum window size
                        if (active_win->x + new_width > 1280) 
                            new_width = 1280 - active_win->x;
                        if (active_win->y + new_height > 800) 
                            new_height = 800 - active_win->y;
                        
                        active_win->width = new_width;
                        active_win->height = new_height;
                    }
                }
            }
            
            // Draw all windows
            for (size_t pi = 0; pi < window_count; pi++)
            {
                Window *win = &windows[pi];
                
                // Draw window - with bounds checking
                for (int h = win->y; h < win->y + win->height && h < 800; h++)
                {
                    if (h < 0) continue;
                    for (int w = win->x; w < win->x + win->width && w < 1280; w++)
                    {
                        if (w < 0) continue;
                        if (wmap[h][w] != pi) continue;
                            
                        // Draw window background
                        (*buf)[h][w][0] = 50;
                        (*buf)[h][w][1] = 50;
                        (*buf)[h][w][2] = 255;
                        
                        // Draw title bar
                        if (h >= win->y && h < win->y + 25)
                        {
                            (*buf)[h][w][0] = 100;
                            (*buf)[h][w][1] = 100;
                            (*buf)[h][w][2] = 255;
                        }
                        // Draw close button (top-right corner of title bar)
                        if(w >= win->x + win->width - 10 && w < win->x + win->width &&
                           h >= win->y && h < win->y + 25){
                            (*buf)[h][w][0] = 255;
                            (*buf)[h][w][1] = 0;
                            (*buf)[h][w][2] = 0;
                        }
                        
                        // Draw resize handle (bottom-right corner)
                        if(w >= win->x + win->width - 10 && w < win->x + win->width &&
                           h >= win->y + win->height - 10 && h < win->y + win->height){
                            (*buf)[h][w][0] = 0;
                            (*buf)[h][w][1] = 255;
                            (*buf)[h][w][2] = 0;  // Make it green to distinguish from close button
                        }
                    }
                }
            }
            
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
        }
        // Draw title text EVERY frame
        for (size_t i = 0; i < window_count; i++)
        {
            // Draw title text character by character for proper clipping
            int title_x = windows[i].x + 5;
            int title_y = windows[i].y + 5;
            
            for (int char_idx = 0; windows[i].title[char_idx] != '\0'; char_idx++)
            {
                char current_char = windows[i].title[char_idx];
                int char_x = title_x + (char_idx * 10);
                int char_y = title_y;
                
                if (char_x >= windows[i].x && char_x < windows[i].x + windows[i].width &&
                    char_y >= windows[i].y && char_y < windows[i].y + 25)
                {
                    if (char_y >= 0 && char_y < 800 && char_x >= 0 && char_x < 1280 && 
                        wmap[char_y][char_x] == i)
                    {
                        drawchar(current_char, char_x, char_y, 255, 255, 255);
                    }
                }
            }
        }
    }
}
