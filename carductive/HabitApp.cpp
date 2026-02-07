#include "HabitApp.h"
#include "Global.h"
#include <M5GFX.h>

extern M5Canvas canvas;

void HabitApp::update() {}

void HabitApp::draw() {
    canvas.setTextColor(COL_ACCENT);
    canvas.setTextSize(2);
    canvas.drawCenterString("HABIT TRACKER", 120, 50);
    
    canvas.setTextSize(1.5);
    canvas.setTextColor(WHITE);
    canvas.drawCenterString("Coming Soon", 120, 80);
}