#ifndef POMODOROAPP_H
#define POMODOROAPP_H

#include <Arduino.h>

struct PomoConfig {
    int workTime;
    int breakTime;
    int volume;
    bool autoStart;
};

class PomodoroApp {
public:
    PomodoroApp();
    void init();
    void update();
    void draw();
    bool isActive();
    void setTask(const char* task);

private:
    bool isRunning;
    bool isBreak;
    int timeLeft;
    unsigned long lastTick;
    
    int volume;
    int workDuration;
    int breakDuration;
    bool autoStart;
    bool showLegend = false;
    char currentTask[64];

    void loadSettings();
    void saveSettings();
    void drawLegendScreen();
};

#endif