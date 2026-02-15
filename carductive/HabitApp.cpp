#include "HabitApp.h"
#include "Global.h"
#include <M5GFX.h>
#include <SD.h>

extern M5Canvas canvas;

void HabitApp::init() {
    loadData();
    if (columns[CAT_MORNING].empty() && columns[CAT_AFTERNOON].empty()) {
        columns[CAT_MORNING].push_back({"Drink Water", false});
        columns[CAT_AFTERNOON].push_back({"Read Book", false});
        columns[CAT_EVENING].push_back({"Sleep 8h", false});
        saveData();
    }
}

bool HabitApp::isTypingMode() { 
    return isTyping; 
}

void HabitApp::addHabit(const char* text) {
    if (text[0] == '\0') return;
    HabitItem newItem;
    strncpy(newItem.text, text, 31);
    newItem.text[31] = '\0';
    newItem.done = false;
    
    columns[currentColumn].push_back(newItem);
    selectedIndex = columns[currentColumn].size() - 1;
    saveData();
}

void HabitApp::deleteHabit() {
    if (columns[currentColumn].empty()) return;
    if (selectedIndex >= 0 && selectedIndex < (int)columns[currentColumn].size()) {
        columns[currentColumn].erase(columns[currentColumn].begin() + selectedIndex);
        if (selectedIndex >= (int)columns[currentColumn].size() && selectedIndex > 0) 
            selectedIndex--;
        saveData();
    }
}

void HabitApp::moveHabit(int direction) {
    int newPos = selectedIndex + direction;
    if (newPos >= 0 && newPos < (int)columns[currentColumn].size()) {
        std::swap(columns[currentColumn][selectedIndex], columns[currentColumn][newPos]);
        selectedIndex = newPos;
        saveData();
    }
}

void HabitApp::toggleHabit() {
    if (columns[currentColumn].empty()) return;
    if (selectedIndex >= 0 && selectedIndex < (int)columns[currentColumn].size()) {
        columns[currentColumn][selectedIndex].done = !columns[currentColumn][selectedIndex].done;
        saveData();
    }
}

void HabitApp::update() {
    if (isTyping) {
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
            addHabit(inputBuffer);
            memset(inputBuffer, 0, sizeof(inputBuffer));
            isTyping = false;
        } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(inputBuffer);
            if (len > 0) inputBuffer[len - 1] = '\0';
        } else if (M5Cardputer.Keyboard.isKeyPressed('`') || M5Cardputer.Keyboard.isKeyPressed(KEY_ESC)) {
            isTyping = false;
        } else {
            for (auto c : M5Cardputer.Keyboard.keysState().word) {
                int len = strlen(inputBuffer);
                if (len < 30) {
                    inputBuffer[len] = c;
                    inputBuffer[len + 1] = '\0';
                }
            }
        }
        return;
    }

    if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
        currentColumn++;
        if (currentColumn >= CAT_COUNT) currentColumn = 0;
        selectedIndex = 0;
    }

    if (M5Cardputer.Keyboard.isKeyPressed(';')) {
        if (selectedIndex > 0) selectedIndex--;
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
        if (selectedIndex < (int)columns[currentColumn].size() - 1) selectedIndex++;
    } else if (M5Cardputer.Keyboard.isKeyPressed('[')) {
        moveHabit(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
        moveHabit(1);
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER) || M5Cardputer.Keyboard.isKeyPressed(' ')) {
        toggleHabit();
    } else if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
        isTyping = true;
        memset(inputBuffer, 0, sizeof(inputBuffer));
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
        deleteHabit();
    }
}

void HabitApp::draw() {
    canvas.fillScreen(COL_BG);

    // 1. GLOWNY NAGLOWEK
    canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
    canvas.setTextColor(COL_ACCENT);
    canvas.setTextSize(1.5);
    canvas.setCursor(5, 4);
    canvas.print("HABIT TRACKER");

    // 2. BELKA KOLUMN
    int tabY = 20;
    int tabH = 22;
    int colW = 240 / 3;

    for (int i = 0; i < 3; i++) {
        uint16_t bg = (i == currentColumn) ? COL_ACCENT : 0x2124; 
        uint16_t txt = (i == currentColumn) ? WHITE : COL_P4;
        
        canvas.fillRect(i * colW, tabY, colW, tabH, bg);
        
        if (i > 0) canvas.drawFastVLine(i * colW, tabY, tabH, COL_BG);
        
        canvas.setTextColor(txt);
        canvas.setTextSize(1.3);
        canvas.setTextDatum(middle_center);
        
        char buf[20];
        snprintf(buf, 20, "%s (%d)", (i==0?"MORN":(i==1?"AFT":"EVE")), (int)columns[i].size());
        
        // ZMIANA: Usunięto "+2" z pozycji Y, aby napisy były ciupineczkę wyżej
        canvas.drawString(buf, (i * colW) + (colW / 2), tabY + (tabH/2));
    }
    canvas.setTextDatum(top_left);

    // 3. LISTA
    int listStartY = 46; 
    
    // ZMIANA: Zmniejszono wysokość wiersza z 22 na 19, aby pasowała do TodoApp
    int itemH = 19; 
    
    int visibleItems = 5; // Przy mniejszym itemH zmieści się więcej (5 zamiast 4)
    
    int offset = 0;
    if (selectedIndex >= visibleItems) offset = selectedIndex - (visibleItems - 1);

    auto& currentList = columns[currentColumn];
    
    if (currentList.empty()) {
        canvas.setTextColor(COL_P4);
        canvas.setTextSize(1.5);
        canvas.drawCenterString("No habits here", 120, 70);
        canvas.setTextSize(1);
        canvas.drawCenterString("Press \\ to add", 120, 90);
    } else {
        for (int i = 0; i < visibleItems; i++) {
            int idx = offset + i;
            if (idx >= (int)currentList.size()) break;

            int y = listStartY + (i * itemH);
            bool isSel = (idx == selectedIndex);
            HabitItem& item = currentList[idx];

            if (isSel) {
                // ZMIANA: Styl podświetlenia skopiowany 1:1 z TodoApp (2, y, 236, 17, 3)
                canvas.fillRoundRect(2, y, 236, 17, 3, COL_HIGHLIGHT);
            }

            // ZMIANA: Checkbox dopasowany do wysokości wiersza (11px), pozycja x=8 jak tekst w Todo
            int boxSize = 11;
            int boxY = y + 3;
            
            canvas.drawRect(8, boxY, boxSize, boxSize, WHITE);
            if (item.done) {
                canvas.fillRect(10, boxY + 2, boxSize - 4, boxSize - 4, COL_ACCENT);
                canvas.setTextColor(COL_TEXT_DONE);
            } else {
                canvas.setTextColor(COL_TEXT_NORM);
            }

            // ZMIANA: Pozycja tekstu y+3 (jak w Todo), x=24 (odsunięcie od kwadratu)
            canvas.setTextSize(1.5);
            canvas.drawString(item.text, 24, y + 3);
        }
    }

    if (isTyping) {
        canvas.fillRect(0, 110, 240, 25, COL_HEADER_BG);
        canvas.drawRect(0, 110, 240, 2, COL_ACCENT);
        canvas.setTextColor(COL_ACCENT);
        canvas.setTextSize(1.5);
        canvas.setCursor(5, 115);
        canvas.printf("> %s_", inputBuffer);
    }
}

void HabitApp::saveData() {
    File f = SD.open(DATA_PATH "/habits_v2.bin", FILE_WRITE);
    if (!f) return;

    for (int i = 0; i < CAT_COUNT; i++) {
        int count = columns[i].size();
        f.write((uint8_t*)&count, sizeof(int));
        for (const auto& item : columns[i]) {
            f.write((uint8_t*)&item, sizeof(HabitItem));
        }
    }
    f.close();
}

void HabitApp::loadData() {
    if (!SD.exists(DATA_PATH "/habits_v2.bin")) return;
    
    File f = SD.open(DATA_PATH "/habits_v2.bin", FILE_READ);
    if (!f) return;

    for (int i = 0; i < CAT_COUNT; i++) {
        columns[i].clear();
        int count = 0;
        f.read((uint8_t*)&count, sizeof(int));
        for (int j = 0; j < count; j++) {
            HabitItem item;
            f.read((uint8_t*)&item, sizeof(HabitItem));
            columns[i].push_back(item);
        }
    }
    f.close();
}