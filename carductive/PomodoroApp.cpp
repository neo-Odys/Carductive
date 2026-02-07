#include "PomodoroApp.h"
#include "Global.h"
#include <M5GFX.h>

extern M5Canvas canvas;

void PomodoroApp::update() {
    // Logic later (Start/Stop timer)
}

void PomodoroApp::draw() {
    // Simple placeholder UI
    canvas.setTextColor(COL_ACCENT);
    canvas.setTextSize(2);
    canvas.drawCenterString("POMODORO", 120, 50);
    
    canvas.setTextSize(1.5);
    canvas.setTextColor(WHITE);
    canvas.drawCenterString("Coming Soon", 120, 80);
}