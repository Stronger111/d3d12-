#pragma once

#include "defines.h"

typedef enum mouse_buttons {
    /** @brief The left mouse button */
    MOUSE_BUTTON_LEFT,
    /** @brief The right mouse button */
    MOUSE_BUTTON_RIGHT,
    /** @brief The middle mouse button (typically the wheel) */
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_MAX
} mouse_buttons;

#define DEFINE_KEY(name, code) KEY_##name = code

typedef enum keys {
    DEFINE_KEY(BACKSPACE, 0x08),
    DEFINE_KEY(ENTER, 0x0D),
    DEFINE_KEY(TAB, 0x09),
    DEFINE_KEY(SHIFT, 0x10),
    DEFINE_KEY(CONTROL, 0x11),

    DEFINE_KEY(PAUSE, 0x13),
    DEFINE_KEY(CAPITAL, 0x14),

    DEFINE_KEY(ESCAPE, 0x1B),

    DEFINE_KEY(CONVERT, 0x1C),
    DEFINE_KEY(NONCONVERT, 0x1D),
    DEFINE_KEY(ACCEPT, 0x1E),
    DEFINE_KEY(MODECHANGE, 0x1F),

    DEFINE_KEY(SPACE, 0x20),
    /** @brief The page up key. */
    KEY_PAGEUP = 0x21,
    /** @brief The page down key. */
    KEY_PAGEDOWN = 0x22,
    DEFINE_KEY(END, 0x23),
    DEFINE_KEY(HOME, 0x24),
    DEFINE_KEY(LEFT, 0x25),
    DEFINE_KEY(UP, 0x26),
    DEFINE_KEY(RIGHT, 0x27),
    DEFINE_KEY(DOWN, 0x28),
    DEFINE_KEY(SELECT, 0x29),
    DEFINE_KEY(PRINT, 0x2A),
    DEFINE_KEY(EXECUTE, 0x2B),
    /** @brief The Print Screen key. */
    KEY_PRINTSCREEN = 0x2C,
    DEFINE_KEY(INSERT, 0x2D),
    DEFINE_KEY(DELETE, 0x2E),
    DEFINE_KEY(HELP, 0x2F),

    /** @brief The 0 key */
    DEFINE_KEY(0, 0x30),
    /** @brief The 1 key */
    KEY_1 = 0x31,
    /** @brief The 2 key */
    KEY_2 = 0x32,
    /** @brief The 3 key */
    KEY_3 = 0x33,
    /** @brief The 4 key */
    KEY_4 = 0x34,
    /** @brief The 5 key */
    KEY_5 = 0x35,
    /** @brief The 6 key */
    KEY_6 = 0x36,
    /** @brief The 7 key */
    KEY_7 = 0x37,
    /** @brief The 8 key */
    KEY_8 = 0x38,
    /** @brief The 9 key */
    KEY_9 = 0x39,

    DEFINE_KEY(A, 0x41),
    DEFINE_KEY(B, 0x42),
    DEFINE_KEY(C, 0x43),
    DEFINE_KEY(D, 0x44),
    DEFINE_KEY(E, 0x45),
    DEFINE_KEY(F, 0x46),
    DEFINE_KEY(G, 0x47),
    DEFINE_KEY(H, 0x48),
    DEFINE_KEY(I, 0x49),
    DEFINE_KEY(J, 0x4A),
    DEFINE_KEY(K, 0x4B),
    DEFINE_KEY(L, 0x4C),
    DEFINE_KEY(M, 0x4D),
    DEFINE_KEY(N, 0x4E),
    DEFINE_KEY(O, 0x4F),
    DEFINE_KEY(P, 0x50),
    DEFINE_KEY(Q, 0x51),
    DEFINE_KEY(R, 0x52),
    DEFINE_KEY(S, 0x53),
    DEFINE_KEY(T, 0x54),
    DEFINE_KEY(U, 0x55),
    DEFINE_KEY(V, 0x56),
    DEFINE_KEY(W, 0x57),
    DEFINE_KEY(X, 0x58),
    DEFINE_KEY(Y, 0x59),
    DEFINE_KEY(Z, 0x5A),

    /** @brief The left Windows/Super key. */
    KEY_LSUPER = 0x5B,
    /** @brief The right Windows/Super key. */
    KEY_RSUPER = 0x5C,
    /** @brief The applicatons key. */
    DEFINE_KEY(APPS, 0x5D),

    DEFINE_KEY(SLEEP, 0x5F),

    DEFINE_KEY(NUMPAD0, 0x60),
    DEFINE_KEY(NUMPAD1, 0x61),
    DEFINE_KEY(NUMPAD2, 0x62),
    DEFINE_KEY(NUMPAD3, 0x63),
    DEFINE_KEY(NUMPAD4, 0x64),
    DEFINE_KEY(NUMPAD5, 0x65),
    DEFINE_KEY(NUMPAD6, 0x66),
    DEFINE_KEY(NUMPAD7, 0x67),
    DEFINE_KEY(NUMPAD8, 0x68),
    DEFINE_KEY(NUMPAD9, 0x69),
    DEFINE_KEY(MULTIPLY, 0x6A),
    DEFINE_KEY(ADD, 0x6B),
    DEFINE_KEY(SEPARATOR, 0x6C),
    DEFINE_KEY(SUBTRACT, 0x6D),
    DEFINE_KEY(DECIMAL, 0x6E),
    DEFINE_KEY(DIVIDE, 0x6F),

    /** @brief The F1 key. */
    DEFINE_KEY(F1, 0x70),
    /** @brief The F2 key. */
    DEFINE_KEY(F2, 0x71),
    /** @brief The F3 key. */
    DEFINE_KEY(F3, 0x72),
    /** @brief The F4 key. */
    DEFINE_KEY(F4, 0x73),
    /** @brief The F5 key. */
    DEFINE_KEY(F5, 0x74),
    /** @brief The F6 key. */
    DEFINE_KEY(F6, 0x75),
    /** @brief The F7 key. */
    DEFINE_KEY(F7, 0x76),
    /** @brief The F8 key. */
    DEFINE_KEY(F8, 0x77),
    /** @brief The F9 key. */
    DEFINE_KEY(F9, 0x78),
    /** @brief The F10 key. */
    DEFINE_KEY(F10, 0x79),
    /** @brief The F11 key. */
    DEFINE_KEY(F11, 0x7A),
    /** @brief The F12 key. */
    DEFINE_KEY(F12, 0x7B),
    /** @brief The F13 key. */
    DEFINE_KEY(F13, 0x7C),
    /** @brief The F14 key. */
    DEFINE_KEY(F14, 0x7D),
    /** @brief The F15 key. */
    DEFINE_KEY(F15, 0x7E),
    /** @brief The F16 key. */
    DEFINE_KEY(F16, 0x7F),
    /** @brief The F17 key. */
    DEFINE_KEY(F17, 0x80),
    /** @brief The F18 key. */
    DEFINE_KEY(F18, 0x81),
    /** @brief The F19 key. */
    DEFINE_KEY(F19, 0x82),
    /** @brief The F20 key. */
    DEFINE_KEY(F20, 0x83),
    /** @brief The F21 key. */
    DEFINE_KEY(F21, 0x84),
    /** @brief The F22 key. */
    DEFINE_KEY(F22, 0x85),
    /** @brief The F23 key. */
    DEFINE_KEY(F23, 0x86),
    /** @brief The F24 key. */
    DEFINE_KEY(F24, 0x87),

    DEFINE_KEY(NUMLOCK, 0x90),
    DEFINE_KEY(SCROLL, 0x91),

    DEFINE_KEY(NUMPAD_EQUAL, 0x92),

    DEFINE_KEY(LSHIFT, 0xA0),
    DEFINE_KEY(RSHIFT, 0xA1),
    DEFINE_KEY(LCONTROL, 0xA2),
    DEFINE_KEY(RCONTROL, 0xA3),
    DEFINE_KEY(LALT, 0xA4),
    DEFINE_KEY(RALT, 0xA5),

    /** @brief The semicolon key. */
    KEY_SEMICOLON = 0x3B,
    /** @brief The apostrophe/single-quote key */
    KEY_APOSTROPHE = 0xDE,
    /** @brief An alias for KEY_APOSTROPHE, apostrophe/single-quote key */
    KEY_QUOTE = KEY_APOSTROPHE,
    /** @brief The equal/plus key. */
    KEY_EQUAL = 0xBB,
    DEFINE_KEY(COMMA, 0xBC),
    DEFINE_KEY(MINUS, 0XBD),
    DEFINE_KEY(PERIOD, 0xBE),
    DEFINE_KEY(SLASH, 0xBF),
    DEFINE_KEY(GRAVE, 0xC0),

    /** @brief The left (square) bracket key e.g. [{ */
    KEY_LBRACKET = 0xDB,
    /** @brief The pipe/backslash key */
    KEY_PIPE = 0xDC,
    /** @brief An alias for the pipe/backslash key */
    KEY_BACKSLASH = KEY_PIPE,
    /** @brief The right (square) bracket key e.g. ]} */
    KEY_RBRACKET = 0xDD,

    KEYS_MAX_KEYS = 0xFF
} keys;
