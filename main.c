#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "key.h"
#include <time.h>
#include "gpu.h"
#include "mouse.h"
#include "wm.h"
int buf[800][1280][3] = {256};
int bgbuf[800][1280][3] = {20};
volatile int running = 1; // Shared variable to control the event loop

void *key_listener_thread(void *arg) {
    listen_for_keys((const char *)arg, &running);
    return NULL;
}
void *mouse_listener_thread(void *arg) {
    listen_mouseEv(&running);
    return NULL;
}

void *gpu_thread_func(void *arg) {
    draw(buf, bgbuf, &running);
    return NULL;
}

void *wm_thread_func(void *arg) {
    wm_init(buf);
    wm_draw(&running);
    return NULL;
}

int main() {
    pthread_t key_thread, gpu_thread, mouse_thread, wm_thread;
    const char *keyboard_device = "/dev/input/event0"; // Replace with your actual device

    // Create threads
    pthread_create(&gpu_thread, NULL, gpu_thread_func, NULL);
    sleep(2);
    system("bash -c clear > /dev/tty2");
    // boot system
    usleep(100000); // 100ms pause to let draw() start
    add_text_message("Welcome to NovaOS!", 10, 10, 255, 255, 255);
    // sleep(2);
    add_text_message("Initialised Framebuffer (gpu.c)", 10, 30, 255, 255, 255);
    pthread_create(&key_thread, NULL, key_listener_thread, (void *)keyboard_device);
    // sleep(2);
    add_text_message("Initialised Keyboard (key.c)", 10, 50, 255, 255, 255);
    pthread_create(&mouse_thread, NULL, mouse_listener_thread, (void *)keyboard_device);
    // sleep(2);
    add_text_message("Initialised Mouse (mouse.c)", 10, 70, 255, 255, 255);
    pthread_create(&wm_thread, NULL, wm_thread_func, NULL);
    // sleep(2);
    add_text_message("Initialised WM (wm.c)", 10, 90, 255, 255, 255);
    
    // Text is now drawn in the render loop to prevent it from being overwritten
    // Wait for threads to finish
    pthread_join(key_thread, NULL);
    pthread_join(mouse_thread, NULL);
    pthread_join(wm_thread, NULL);
    pthread_join(gpu_thread, NULL);

    printf("Program exited.\n");
    return 0;
}

