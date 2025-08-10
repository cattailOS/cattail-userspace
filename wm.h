#ifndef WM_H
#define WM_H

// Forward declaration of Window struct
typedef struct
{
    int x, y;
    int width, height;
    char title[64];
    int owner_id;
    int win_id;
    int ***buffer; // [height][width][3]
} Window;

// External declarations for global variables
extern Window *windows;
extern size_t window_count;

// Function declarations
void wm_init(int bufptr[800][1280][3]);
void wm_draw(volatile int *running);

#endif // WM_H

// dabatases (test commit for codacy to give status checks)
// motherfucker its supposed to be PR not push
// fucker