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
public: // <-- TE FUNKCJE MUSZĄ BYĆ TUTAJ
    void init();
    void update();
    void draw();
    bool isTypingMode();

    bool moveColumnLeft();
    bool moveColumnRight();
    void setColumn(int col);

private: // <-- A ZMIENNE DOPIERO TUTAJ
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
    
    void saveMaster(); 
    void loadMaster();
    
    void loadDay();
    void appendHabitState(int cat, const HabitItem& item);
    void saveLastDate();
};

#endif