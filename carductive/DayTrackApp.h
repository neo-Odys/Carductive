// ==================== DayTrackApp.h ====================
#ifndef DAYTRACKAPP_H
#define DAYTRACKAPP_H

#include <Arduino.h>

class DayTrackApp {
public:
    void init();
    void update();
    void draw();

private:
    int daySchedule[24];
    bool showLegend = false;
    
    uint16_t getCatColor(int id);
    const char* getCatName(int id);

    void changeDate(int delta);
    void loadForCurrentDate();
    void saveForCurrentDate();

    void drawTimeline();
    void drawLegendScreen();
};

#endif