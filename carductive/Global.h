#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <M5Cardputer.h>

#ifndef KEY_ESC
#define KEY_ESC 27
#endif
#ifndef KEY_TAB
#define KEY_TAB 9
#endif

enum AppMode {
    APP_TODO,
    APP_POMODORO,
    APP_HABIT,
    APP_DAY,
    APP_COUNT
};

#define BRIGHT_HIGH  80
#define BRIGHT_DIM   6
#define DIM_DELAY_MS 15000

#define COL_BG          0x10A2
#define COL_HEADER_BG   0x0841

#define COL_HIGHLIGHT   0x4A69
#define COL_SEPARATOR   0xFFFF

#define COL_TEXT_NORM   0xFFFF
#define COL_TEXT_DONE   0x7BEF

#define COL_P1          0xF448
#define COL_P2          0xFDC0
#define COL_P3          0x8CD9
#define COL_P4          0xBDF7

#define COL_ACCENT      COL_P3
#define COL_REORDER     COL_P3

#define DATA_PATH "/Carductive"

#endif