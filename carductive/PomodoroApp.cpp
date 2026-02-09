#include "PomodoroApp.h"
#include "Global.h"
#include <M5GFX.h>

extern M5Canvas canvas;

PomodoroApp::PomodoroApp() {
    isRunning = false;
    timeLeft = 25 * 60;
    lastTick = 0;
}

bool PomodoroApp::isActive() {
    return isRunning;
}

void PomodoroApp::update() {
    static bool prevEnter = false;
    static bool prevBksp = false;

    bool currEnter = M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER);
    bool currBksp = M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE);

    // START / STOP
    if (currEnter && !prevEnter) {
        isRunning = !isRunning;
        M5.Speaker.tone(1000, 100);
        lastTick = millis(); 
    }
    prevEnter = currEnter;

    if (currBksp && !prevBksp) {
        isRunning = false;
        timeLeft = 25 * 60;
        M5.Speaker.tone(800, 50);
    }
    prevBksp = currBksp;

    if (isRunning && timeLeft > 0) {
        if (millis() - lastTick >= 1000) {
            timeLeft--;
            lastTick = millis();
            
            if (timeLeft <= 0) {
                isRunning = false;
                
                draw(); 
                canvas.pushSprite(0,0);

                M5.Speaker.tone(2000, 200);
                delay(300);
                M5.Speaker.tone(2000, 400);
            }
        }
    }
}

void PomodoroApp::draw() {
    canvas.setTextColor(COL_ACCENT);
    canvas.setTextSize(1.5);
    canvas.drawCenterString("POMODORO", 120, 10);

    int minutes = timeLeft / 60;
    int seconds = timeLeft % 60;
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);

    canvas.setTextSize(5);
    
    if (timeLeft == 0) canvas.setTextColor(TFT_RED);
    else if (isRunning) canvas.setTextColor(TFT_GREEN);
    else canvas.setTextColor(COL_TEXT_NORM);
    
    canvas.drawCenterString(timeStr, 120, 45);

    canvas.setTextSize(1.2);
    canvas.setTextColor(COL_P4);
    if (isRunning) {
        canvas.drawCenterString("[Enter] Pause (Locked)", 120, 110);
    } else {
        canvas.drawCenterString("[Enter] Start   [BKSP] Reset", 120, 110);
    }
}