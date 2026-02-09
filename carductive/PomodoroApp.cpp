#include "PomodoroApp.h"
#include "Global.h"
#include <M5GFX.h>
#include <SD.h>

extern M5Canvas canvas;

PomodoroApp::PomodoroApp() {
    isRunning = false;
    isBreak = false;
    lastTick = 0;
    workDuration = 25 * 60;
    breakDuration = 5 * 60;
    volume = 100;
    autoStart = false;
    timeLeft = workDuration;
}

void PomodoroApp::init() {
    loadSettings();
    timeLeft = workDuration;
    M5.Speaker.setVolume(volume);
}

bool PomodoroApp::isActive() {
    return isRunning;
}

void PomodoroApp::loadSettings() {
    if (SD.exists(DATA_PATH "/pomo.bin")) {
        File f = SD.open(DATA_PATH "/pomo.bin", FILE_READ);
        if (f) {
            PomoConfig cfg;
            if (f.read((uint8_t*)&cfg, sizeof(PomoConfig)) == sizeof(PomoConfig)) {
                workDuration = cfg.workTime;
                breakDuration = cfg.breakTime;
                volume = cfg.volume;
                autoStart = cfg.autoStart;
            }
            f.close();
        }
    }
}

void PomodoroApp::saveSettings() {
    PomoConfig cfg;
    cfg.workTime = workDuration;
    cfg.breakTime = breakDuration;
    cfg.volume = volume;
    cfg.autoStart = autoStart;

    File f = SD.open(DATA_PATH "/pomo.bin", FILE_WRITE);
    if (f) {
        f.write((uint8_t*)&cfg, sizeof(PomoConfig));
        f.close();
    }
}

void PomodoroApp::update() {
    static bool prevEnter = false;
    static bool prevBksp = false;
    static bool prevUp = false;
    static bool prevDown = false;
    static bool prevLBracket = false;
    static bool prevRBracket = false;
    static bool prevTab = false;
    static bool prevA = false;

    bool kEnter = M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER);
    bool kBksp  = M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE);
    bool kUp    = M5Cardputer.Keyboard.isKeyPressed(';');
    bool kDown  = M5Cardputer.Keyboard.isKeyPressed('.');
    bool kLeftBr = M5Cardputer.Keyboard.isKeyPressed('[');
    bool kRightBr = M5Cardputer.Keyboard.isKeyPressed(']');
    bool kTab   = M5Cardputer.Keyboard.isKeyPressed(KEY_TAB);
    bool kA     = M5Cardputer.Keyboard.isKeyPressed('a');

    if (kEnter && !prevEnter) {
        isRunning = !isRunning;
        M5.Speaker.tone(1000, 100);
        lastTick = millis(); 
    }
    prevEnter = kEnter;

    if (kBksp && !prevBksp) {
        isRunning = false;
        timeLeft = isBreak ? breakDuration : workDuration;
        M5.Speaker.tone(800, 50);
    }
    prevBksp = kBksp;

    if (kTab && !prevTab) {
        isBreak = !isBreak;
        isRunning = false;
        timeLeft = isBreak ? breakDuration : workDuration;
        M5.Speaker.tone(1500, 50);
    }
    prevTab = kTab;

    if (kA && !prevA) {
        autoStart = !autoStart;
        saveSettings();
        M5.Speaker.tone(autoStart ? 1200 : 800, 50);
    }
    prevA = kA;

    bool volChanged = false;
    if (kUp && !prevUp) {
        volume += 25;
        if (volume > 255) volume = 255;
        volChanged = true;
        M5.Speaker.tone(1200, 20);
    }
    prevUp = kUp;

    if (kDown && !prevDown) {
        volume -= 25;
        if (volume < 0) volume = 0;
        volChanged = true;
        M5.Speaker.tone(800, 20);
    }
    prevDown = kDown;

    if (volChanged) {
        M5.Speaker.setVolume(volume);
        saveSettings();
    }

    int timeStep = 5 * 60;
    bool timeChanged = false;

    if (kLeftBr && !prevLBracket) {
        timeLeft -= timeStep;
        if (timeLeft < 0) timeLeft = 0;
        if (isBreak) {
            breakDuration -= timeStep;
            if (breakDuration < 0) breakDuration = 0;
        } else {
            workDuration -= timeStep;
            if (workDuration < 0) workDuration = 0;
        }
        timeChanged = true;
        M5.Speaker.tone(600, 50);
    }
    prevLBracket = kLeftBr;

    if (kRightBr && !prevRBracket) {
        timeLeft += timeStep;
        if (isBreak) breakDuration += timeStep;
        else workDuration += timeStep;
        timeChanged = true;
        M5.Speaker.tone(1400, 50);
    }
    prevRBracket = kRightBr;

    if (timeChanged) {
        saveSettings();
    }

    if (isRunning && timeLeft > 0) {
        if (millis() - lastTick >= 1000) {
            timeLeft--;
            lastTick = millis();
            
            if (timeLeft <= 0) {
                M5.Speaker.tone(2000, 200);
                delay(300);
                M5.Speaker.tone(2000, 400);
                
                isBreak = !isBreak;
                timeLeft = isBreak ? breakDuration : workDuration;
                isRunning = autoStart; 
            }
        }
    }
}

void PomodoroApp::draw() {
    canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
    
    canvas.setTextSize(1.5);
    canvas.setCursor(5, 5);
    canvas.setTextColor(COL_TEXT_NORM);
    canvas.print("POMODORO ");
    
    if (isBreak) {
        canvas.setTextColor(COL_P3);
        canvas.print("[BREAK]");
    } else {
        canvas.setTextColor(COL_ACCENT);
        canvas.print("[WORK]");
    }

    int minutes = timeLeft / 60;
    int seconds = timeLeft % 60;
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);

    canvas.setTextSize(5);
    if (isBreak) {
        if (isRunning) canvas.setTextColor(COL_P3);
        else canvas.setTextColor(WHITE);
    } else {
        if (isRunning) canvas.setTextColor(TFT_GREEN);
        else canvas.setTextColor(COL_TEXT_NORM);
    }
    
    canvas.drawCenterString(timeStr, 110, 45);

    canvas.setTextSize(1.2);
    canvas.setTextColor(COL_P4);
    
    int instrY = 100;
    
    if (isRunning) {
        canvas.drawCenterString("[Enter] Pause", 110, instrY);
    } else {
        canvas.drawCenterString("[Enter] Start  [Tab] Mode", 110, instrY);
    }

    canvas.setTextColor(COL_TEXT_NORM);
    canvas.setTextSize(1.0);
    canvas.drawCenterString("-5m [   ] +5m   (Bksp: Rst)", 110, instrY + 15);

    canvas.setCursor(5, 122);
    if (autoStart) {
        canvas.setTextColor(TFT_GREEN);
        canvas.print("AUTO: ON [A]");
    } else {
        canvas.setTextColor(COL_P1);
        canvas.print("AUTO: OFF [A]");
    }

    int barX = 225;
    int barY = 30;
    int barW = 8;
    int barH = 80;

    canvas.drawRect(barX, barY, barW, barH, COL_TEXT_NORM);
    
    int volHeight = map(volume, 0, 255, 0, barH - 2);
    if (volHeight > 0) {
        canvas.fillRect(barX + 1, barY + barH - 1 - volHeight, barW - 2, volHeight, COL_ACCENT);
    }

    canvas.setTextSize(1);
    canvas.setTextColor(COL_P4);
    canvas.setTextDatum(top_center);
    canvas.drawString("VOL", barX + (barW / 2), barY - 9);
    canvas.setTextDatum(top_left);
}