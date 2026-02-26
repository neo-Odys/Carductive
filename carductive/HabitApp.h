#ifndef HABITAPP_H
#define HABITAPP_H

#include <Arduino.h>
#include <vector>

enum HabitCat { CAT_MORNING, CAT_AFTERNOON, CAT_EVENING, CAT_COUNT };

struct HabitItem {
    char text[32]; 
    bool done;
};

class HabitApp {
public:
    void init();
    void update();
    void draw();
    bool isTypingMode();

private:
    std::vector<HabitItem> columns[CAT_COUNT];
    
    int currentColumn = 0;
    int selectedIndex = 0;
    
    bool isTyping = false;
    char inputBuffer[32] = {0};
    
    int viewDay = 26;
    int viewMonth = 2;
    int viewYear = 2026;
    
    const char* catNames[CAT_COUNT] = {"MORNING", "AFTERNOON", "EVENING"};

    void toggleHabit();
    void addHabit(const char* text);
    void deleteHabit();
    void moveHabit(int direction); 
    void changeDate(int delta);
    void getFilePath(char* buffer);
    
    void saveMaster(); 
    void loadMaster();
    void saveDay();
    void loadDay();
};

#endif