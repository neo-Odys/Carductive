#ifndef HABITAPP_H
#define HABITAPP_H

#include <Arduino.h>
#include <vector>

enum HabitCat { CAT_MORNING, CAT_AFTERNOON, CAT_EVENING, CAT_COUNT };

struct HabitItem {
    char text[32]; // Więcej miejsca na tekst
    bool done;
};

class HabitApp {
public:
    void init();
    void update();
    void draw();
    bool isTypingMode();

private:
    // Tablica wektorów - 3 oddzielne listy
    std::vector<HabitItem> columns[CAT_COUNT];
    
    int currentColumn = 0; // 0=Morning, 1=Afternoon, 2=Evening
    int selectedIndex = 0;
    
    bool isTyping = false;
    char inputBuffer[32] = {0};
    
    const char* catNames[CAT_COUNT] = {"MORNING", "AFTERNOON", "EVENING"};

    void toggleHabit();
    void addHabit(const char* text);
    void deleteHabit();
    void moveHabit(int direction); // -1 góra, 1 dół
    
    // Prosty zapis binarny (zeby nie traciec danych po restarcie)
    void saveData(); 
    void loadData();
};

#endif