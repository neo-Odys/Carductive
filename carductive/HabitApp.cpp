#include "HabitApp.h"
#include "Global.h"
#include <M5GFX.h>
#include <SD.h>

extern M5Canvas canvas;

void HabitApp::getFilePath(char* buffer) {
    sprintf(buffer, DATA_PATH "/h_%04d%02d%02d.csv", viewYear, viewMonth, viewDay);
}

void HabitApp::init() {
    bool isFirstRun = !SD.exists(DATA_PATH "/habit_lastdate.bin");
    
    if (!isFirstRun) {
        File sf = SD.open(DATA_PATH "/habit_lastdate.bin", FILE_READ);
        if (sf) {
            sf.read((uint8_t*)&viewYear, sizeof(int));
            sf.read((uint8_t*)&viewMonth, sizeof(int));
            sf.read((uint8_t*)&viewDay, sizeof(int));
            sf.close();
        }
    }
    
    // Ładowanie listy globalnej (Master List)
    if (!SD.exists(DATA_PATH "/habits_master.csv")) {
        columns[CAT_MORNING].push_back({"Drink Water", false});
        columns[CAT_AFTERNOON].push_back({"Read Book", false});
        columns[CAT_EVENING].push_back({"Sleep 8h", false});
        saveMaster();
    } else {
        loadMaster();
    }
    
    // Nałożenie zaznaczeń z dzisiejszego dnia na listę globalną
    loadDay();
    saveDay(); 
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
    
    saveMaster(); // Zapisz na globalną listę
    saveDay();    // Zapisz aktualny dzień
}

void HabitApp::deleteHabit() {
    if (columns[currentColumn].empty()) return;
    if (selectedIndex >= 0 && selectedIndex < (int)columns[currentColumn].size()) {
        columns[currentColumn].erase(columns[currentColumn].begin() + selectedIndex);
        if (selectedIndex >= (int)columns[currentColumn].size() && selectedIndex > 0) 
            selectedIndex--;
            
        saveMaster(); // Usuwa z globalnej listy (znika wszędzie)
        saveDay();
    }
}

void HabitApp::moveHabit(int direction) {
    int newPos = selectedIndex + direction;
    if (newPos >= 0 && newPos < (int)columns[currentColumn].size()) {
        std::swap(columns[currentColumn][selectedIndex], columns[currentColumn][newPos]);
        selectedIndex = newPos;
        saveMaster(); // Przesunięcie jest globalne
        saveDay();
    }
}

void HabitApp::changeDate(int delta) {
    saveDay(); // Najpierw zapisz obecny dzień
    
    viewDay += delta;
    int daysInMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (viewYear % 4 == 0 && (viewYear % 100 != 0 || viewYear % 400 == 0)) daysInMonth[2] = 29;
    
    if (viewDay > daysInMonth[viewMonth]) {
        viewDay = 1;
        viewMonth++;
        if (viewMonth > 12) { viewMonth = 1; viewYear++; }
    } else if (viewDay < 1) {
        viewMonth--;
        if (viewMonth < 1) { viewMonth = 12; viewYear--; }
        viewDay = daysInMonth[viewMonth];
    }
    
    loadDay(); // Wczytaj odznaczenia (checkboxy) dla nowego dnia
    selectedIndex = 0;
}

void HabitApp::toggleHabit() {
    if (columns[currentColumn].empty()) return;
    if (selectedIndex >= 0 && selectedIndex < (int)columns[currentColumn].size()) {
        columns[currentColumn][selectedIndex].done = !columns[currentColumn][selectedIndex].done;
        saveDay(); // Zaznaczenia zapisujemy TYLKO do tego konkretnego dnia
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
                    if (c != ';') {
                        inputBuffer[len] = c;
                        inputBuffer[len + 1] = '\0';
                    }
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
        changeDate(-1); 
    } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
        changeDate(1);  
    } else if (M5Cardputer.Keyboard.isKeyPressed('-')) {
        moveHabit(-1);  
    } else if (M5Cardputer.Keyboard.isKeyPressed('=')) {
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

    canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
    
    canvas.setTextColor(WHITE);
    canvas.setTextSize(1.5);
    canvas.setCursor(5, 4);
    canvas.print("HABITS");

    char dateStr[20];
    snprintf(dateStr, sizeof(dateStr), "[%02d.%02d.%02d]", viewDay, viewMonth, viewYear % 100);
    canvas.setTextColor(COL_ACCENT);
    
    // DATA BARDZIEJ NA LEWO (z 235 na 205)
    canvas.drawRightString(dateStr, 180, 4); 

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
        
        canvas.drawString(buf, (i * colW) + (colW / 2), tabY + (tabH/2));
    }
    canvas.setTextDatum(top_left);

    int listStartY = 46; 
    int itemH = 19; 
    int visibleItems = 5; 
    
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
                canvas.fillRoundRect(2, y, 236, 17, 3, COL_HIGHLIGHT);
            }

            int boxSize = 11;
            int boxY = y + 3;
            
            canvas.drawRect(8, boxY, boxSize, boxSize, WHITE);
            if (item.done) {
                canvas.fillRect(10, boxY + 2, boxSize - 4, boxSize - 4, COL_ACCENT);
                canvas.setTextColor(COL_TEXT_DONE);
            } else {
                canvas.setTextColor(COL_TEXT_NORM);
            }

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

void HabitApp::saveMaster() {
    if (SD.exists(DATA_PATH "/habits_master.csv")) {
        SD.remove(DATA_PATH "/habits_master.csv");
    }
    
    File f = SD.open(DATA_PATH "/habits_master.csv", FILE_WRITE);
    if (!f) return;

    f.println("ID_Kategorii;Nazwa_Nawyku");
    for (int i = 0; i < CAT_COUNT; i++) {
        for (const auto& item : columns[i]) {
            f.printf("%d;%s\n", i, item.text);
        }
    }
    f.close();
}

void HabitApp::loadMaster() {
    for (int i = 0; i < CAT_COUNT; i++) {
        columns[i].clear();
    }
    
    if (!SD.exists(DATA_PATH "/habits_master.csv")) return;
    
    File f = SD.open(DATA_PATH "/habits_master.csv", FILE_READ);
    if (!f) return;

    bool isFirstLine = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (isFirstLine) {
            isFirstLine = false; 
            continue;
        }

        int firstSemi = line.indexOf(';');
        if (firstSemi == -1) continue;

        int cat = line.substring(0, firstSemi).toInt();
        String textStr = line.substring(firstSemi + 1);

        if (cat >= 0 && cat < CAT_COUNT) {
            HabitItem item;
            strncpy(item.text, textStr.c_str(), 31);
            item.text[31] = '\0';
            item.done = false; // Ładujemy zmasterowaną listę zawsze jako niezrobioną
            columns[cat].push_back(item);
        }
    }
    f.close();
}

void HabitApp::saveDay() {
    char path[64];
    getFilePath(path);
    
    if (SD.exists(path)) {
        SD.remove(path);
    }
    
    File f = SD.open(path, FILE_WRITE);
    if (f) {
        f.println("ID_Kategorii;Czy_Zrobione;Nazwa_Nawyku");
        for (int i = 0; i < CAT_COUNT; i++) {
            for (const auto& item : columns[i]) {
                int isDone = item.done ? 1 : 0;
                f.printf("%d;%d;%s\n", i, isDone, item.text);
            }
        }
        f.close();
    }
    
    File sf = SD.open(DATA_PATH "/habit_lastdate.bin", FILE_WRITE);
    if (sf) {
        sf.write((uint8_t*)&viewYear, sizeof(int));
        sf.write((uint8_t*)&viewMonth, sizeof(int));
        sf.write((uint8_t*)&viewDay, sizeof(int));
        sf.close();
    }
}

void HabitApp::loadDay() {
    // 1. Zresetuj wszystkie checkboxy na liście globalnej na niezaznaczone
    for (int i = 0; i < CAT_COUNT; i++) {
        for (auto& item : columns[i]) {
            item.done = false;
        }
    }

    char path[64];
    getFilePath(path);
    
    // Jeśli nie ma pliku na ten dzień, nawyki pozostają na liście jako "niezrobione"
    if (!SD.exists(path)) return; 
    
    File f = SD.open(path, FILE_READ);
    if (!f) return;

    bool isFirstLine = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (isFirstLine) {
            isFirstLine = false; 
            continue;
        }

        int firstSemi = line.indexOf(';');
        if (firstSemi == -1) continue;
        int secondSemi = line.indexOf(';', firstSemi + 1);
        if (secondSemi == -1) continue;

        int cat = line.substring(0, firstSemi).toInt();
        int done = line.substring(firstSemi + 1, secondSemi).toInt();
        String textStr = line.substring(secondSemi + 1);

        if (cat >= 0 && cat < CAT_COUNT) {
            // Szukamy nawyku o tej nazwie w aktualnej głównej liście 
            for (auto& item : columns[cat]) {
                if (String(item.text) == textStr) {
                    item.done = (done == 1); // Zaznaczamy checkbox jeśli w pliku z dnia był zrobiony
                    break;
                }
            }
        }
    }
    f.close();
}