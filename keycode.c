#include <stdio.h>
#include "keycode.h"
#define MAX_KEYCODE 256

char *key_names[MAX_KEYCODE];

void init_key_names() {
    for (int i = 0; i < MAX_KEYCODE; i++) {
        key_names[i] = "UNKNOWN";
    }

    // Letters
    key_names[16] = "Q";
    key_names[17] = "W";
    key_names[18] = "E";
    key_names[19] = "R";
    key_names[20] = "T";
    key_names[21] = "Y";
    key_names[22] = "U";
    key_names[23] = "I";
    key_names[24] = "O";
    key_names[25] = "P";

    key_names[30] = "A";
    key_names[31] = "S";
    key_names[32] = "D";
    key_names[33] = "F";
    key_names[34] = "G";
    key_names[35] = "H";
    key_names[36] = "J";
    key_names[37] = "K";
    key_names[38] = "L";

    key_names[44] = "Z";
    key_names[45] = "X";
    key_names[46] = "C";
    key_names[47] = "V";
    key_names[48] = "B";
    key_names[49] = "N";
    key_names[50] = "M";

    // Numbers (top row)
    key_names[2] = "1";
    key_names[3] = "2";
    key_names[4] = "3";
    key_names[5] = "4";
    key_names[6] = "5";
    key_names[7] = "6";
    key_names[8] = "7";
    key_names[9] = "8";
    key_names[10] = "9";
    key_names[11] = "0";

    // Symbols and punctuation
    key_names[12] = "MINUS";
    key_names[13] = "EQUAL";
    key_names[26] = "LEFTBRACE";    // [
    key_names[27] = "RIGHTBRACE";   // ]
    key_names[39] = "SEMICOLON";
    key_names[40] = "APOSTROPHE";
    key_names[41] = "GRAVE";        // `
    key_names[43] = "BACKSLASH";
    key_names[51] = "COMMA";
    key_names[52] = "DOT";
    key_names[53] = "SLASH";

    // Control keys
    key_names[1] = "ESC";
    key_names[14] = "BACKSPACE";
    key_names[15] = "TAB";
    key_names[28] = "ENTER";
    key_names[29] = "LEFTCTRL";
    key_names[42] = "LEFTSHIFT";
    key_names[54] = "RIGHTSHIFT";
    key_names[56] = "LEFTALT";
    key_names[100] = "RIGHTALT";    // AltGr
    key_names[125] = "LEFTMETA";    // Windows key / Command key
    key_names[126] = "RIGHTMETA";

    key_names[57] = "SPACE";
    key_names[58] = "CAPSLOCK";

    // Function keys
    key_names[59] = "F1";
    key_names[60] = "F2";
    key_names[61] = "F3";
    key_names[62] = "F4";
    key_names[63] = "F5";
    key_names[64] = "F6";
    key_names[65] = "F7";
    key_names[66] = "F8";
    key_names[67] = "F9";
    key_names[68] = "F10";
    key_names[87] = "F11";
    key_names[88] = "F12";

    // Navigation keys
    key_names[103] = "UP";
    key_names[108] = "DOWN";
    key_names[105] = "RIGHT";
    key_names[106] = "LEFT";
    key_names[111] = "DELETE";
    key_names[110] = "END";
    key_names[115] = "HOME";
    key_names[119] = "PAGEUP";
    key_names[117] = "PAGEDOWN";

    // Keypad
    key_names[79] = "KP_1";
    key_names[80] = "KP_2";
    key_names[81] = "KP_3";
    key_names[82] = "KP_4";
    key_names[83] = "KP_5";
    key_names[84] = "KP_6";
    key_names[85] = "KP_7";
    key_names[86] = "KP_8";
    key_names[87] = "KP_9";
    key_names[88] = "KP_0";
    key_names[96] = "KP_ENTER";

    // Misc
    key_names[70] = "SCROLLLOCK";
    key_names[69] = "NUMLOCK";

    // Add more keys here as you need
}

char *keycode_to_name(int code) {
    if (code < 0 || code >= MAX_KEYCODE) {
        return "INVALID";
    }
    return key_names[code];
}