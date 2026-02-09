#ifndef POMODOROAPP_H
#define POMODOROAPP_H

#include <Arduino.h>

class PomodoroApp {
public:
    PomodoroApp();
    void update();
    void draw();
    bool isActive();

private:
    bool isRunning;
    int timeLeft;
    unsigned long lastTick;
    
    const int WORK_TIME = 25 * 60;
};

#endif