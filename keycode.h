// keymap_us_qwerty.h
#ifndef KEYMAP_US_QWERTY_H
#define KEYMAP_US_QWERTY_H

#define MAX_KEYCODE 256

// Initialize the key name mappings
void init_key_names(void);

// Convert a keycode to a human-readable name string
char *keycode_to_name(int code);

#endif // KEYMAP_US_QWERTY_H
