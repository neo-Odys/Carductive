#include "DayTrackApp.h"
#include "Global.h"
#include <M5GFX.h>
#include <SD.h>

extern M5Canvas canvas;

#define CAT_UNI          0x6596 // Jasny niebieski
#define CAT_STUDY        0x5bba // Ciemniejszy, żywszy niebieski
#define CAT_PROJ         0x766f // Świeży zielony
#define CAT_PROD         0x6673 // Morski / Teal
#define CAT_FAM          0xd34f // Ciepły pomarańczowy
#define CAT_FRIEND       0xe70b // Miodowo-żółty
#define CAT_CALISTHENICS 0xb3da // Oliwkowo-żółty 
#define CAT_SPORT        0xFCEB // Koralowy / Brzoskwiniowy
#define CAT_ROUT         0xb538 // Fioletowy / Lawendowy
#define CAT_CHORES       0xbc79 // Różowo-fioletowy
#define CAT_READ         0x8dbc // Chłodny błękit
#define CAT_RELAX        0xdc99 // Pudrowy róż
#define CAT_GAMES        0x6e7c // Błękitny / Cyjan
#define CAT_WASTE        0xf800 // Szary
#define CAT_TRANSIT      0xBC2D // Cynamonowy / Zgaszony brąz
#define CAT_SLEEP        0x9CD3 // Ciemniejszy szary / Stalowy
#define CAT_EMPTY        0x0000 // Czarny

void DayTrackApp::init() {
    loadForCurrentDate();
}

uint16_t DayTrackApp::getCatColor(int id) {
    switch(id) {
        case 1: return CAT_UNI;          case 2: return CAT_STUDY;
        case 3: return CAT_PROJ;         case 4: return CAT_PROD;
        case 5: return CAT_FAM;          case 6: return CAT_FRIEND;
        case 7: return CAT_CALISTHENICS; 
        case 8: // Fallback dla usuniętego RUN
        case 9: return CAT_SPORT;   
        case 10: return CAT_ROUT;        case 11: return CAT_CHORES;
        case 12: return CAT_READ;        case 13: return CAT_RELAX;
        case 14: return CAT_GAMES;
        case 15: 
        case 16: // Fallback dla usuniętego SCROLL
            return CAT_WASTE;  
        case 17: 
        case 18: // Fallback dla usuniętego NAP
            return CAT_SLEEP;
        case 19: return CAT_TRANSIT;
        default: return CAT_EMPTY;
    }
}

const char* DayTrackApp::getCatName(int id) {
    switch(id) {
        case 1: return "UNI";            case 2: return "STUDY";
        case 3: return "PROJECT";        case 4: return "PRODUCTIVE";
        case 5: return "FAMILY";         case 6: return "FRIENDS";
        case 7: return "CALISTHENICS";   
        case 8: // Fallback
        case 9: return "SPORT";     
        case 10: return "ROUTINE";       case 11: return "CHORES";
        case 12: return "READ";          case 13: return "RELAX";    
        case 14: return "GAMES";
        case 15: 
        case 16: // Fallback
            return "WASTE";    
        case 17: 
        case 18: // Fallback
            return "SLEEP";    
        case 19: return "TRANSIT";
        default: return "EMPTY";
    }
}

void DayTrackApp::changeDate(int delta) {
    globalDay += delta;
    int daysInMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (globalYear % 4 == 0 && (globalYear % 100 != 0 || globalYear % 400 == 0)) daysInMonth[2] = 29;
    
    if (globalDay > daysInMonth[globalMonth]) {
        globalDay = 1; globalMonth++;
        if (globalMonth > 12) { globalMonth = 1; globalYear++; }
    } else if (globalDay < 1) {
        globalMonth--;
        if (globalMonth < 1) { globalMonth = 12; globalYear--; }
        globalDay = daysInMonth[globalMonth];
    }
    
    saveGlobalDate();
    loadForCurrentDate();
}

void DayTrackApp::loadForCurrentDate() {
    for(int i = 0; i < 24; i++) daySchedule[i] = 0;
    for(int i = 0; i <= 6; i++) daySchedule[i] = 17;
    daySchedule[23] = 17;

    char path[64];
    snprintf(path, sizeof(path), DATA_PATH "/day_%04d%02d%02d.csv", globalYear, globalMonth, globalDay);
    if (!SD.exists(path)) return;
    
    File f = SD.open(path, FILE_READ);
    if (!f) return;

    bool isFirstLine = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (isFirstLine) { isFirstLine = false; continue; }

        int semiIndex = line.indexOf(';');
        if (semiIndex != -1) {
            int h = line.substring(0, semiIndex).toInt();
            int cat = line.substring(semiIndex + 1).toInt();
            if (h >= 0 && h < 24) daySchedule[h] = cat;
        }
    }
    f.close();
}

void DayTrackApp::saveForCurrentDate() {
    char path[64];
    snprintf(path, sizeof(path), DATA_PATH "/day_%04d%02d%02d.csv", globalYear, globalMonth, globalDay);
    File f = SD.open(path, FILE_WRITE);
    if (f) {
        f.println("Hour;CategoryID");
        for (int i = 0; i < 24; i++) {
            f.printf("%d;%d\n", i, daySchedule[i]);
        }
        f.close();
    }
}

void DayTrackApp::update() {
    if (showLegend) {
        if (M5Cardputer.Keyboard.isKeyPressed('l') || 
            M5Cardputer.Keyboard.isKeyPressed('`') || 
            M5Cardputer.Keyboard.isKeyPressed(KEY_ESC)) {
            showLegend = false;
        }
        return;
    }

    bool changed = false;

    if (M5Cardputer.Keyboard.isKeyPressed(';')) { 
        if (globalHour < 23) {
            globalHour++;
            saveGlobalDate();
        }
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) { 
        if (globalHour > 0) {
            globalHour--;
            saveGlobalDate();
        }
    } 
    else if (M5Cardputer.Keyboard.isKeyPressed('[')) {
        changeDate(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
        changeDate(1);
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('1')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 1) ? 2 : 1;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('2')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 3) ? 4 : 3;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('3')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 5) ? 6 : 5;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('4')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 7) ? 9 : 7;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('5')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 10) ? 11 : 10;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('6')) {
        int c = daySchedule[globalHour];
        if (c == 12) daySchedule[globalHour] = 13;
        else if (c == 13) daySchedule[globalHour] = 14;
        else daySchedule[globalHour] = 12;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('7')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 15) ? 19 : 15;
        changed = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('8')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 17) ? 0 : 17;
        changed = true;
    } 
    else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
        daySchedule[globalHour] = 0;
        changed = true;
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('l')) {
        showLegend = true;
    }

    if (changed) {
        saveForCurrentDate();
    }
}

void DayTrackApp::draw() {
    if (showLegend) {
        drawLegendScreen();
        return;
    }

    canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
    canvas.setTextSize(1.5);
    canvas.setCursor(5, 4);
    canvas.setTextColor(WHITE);
    canvas.print("DAY TRACK"); 

    char dateStr[20];
    snprintf(dateStr, sizeof(dateStr), "[%02d.%02d.%02d]", globalDay, globalMonth, globalYear % 100);
    
    canvas.setTextColor(COL_ACCENT);
    canvas.drawRightString(dateStr, 190, 4); 

    drawTimeline();
}

void DayTrackApp::drawTimeline() {
    int boxW = 34;
    int boxH = 40;
    int startY = 48; 
    int gap = 8;
    int centerX = 120; 

    for (int offset = -2; offset <= 2; offset++) {
        int h = globalHour + offset;
        int x = centerX - (boxW / 2) + (offset * (boxW + gap));

        if (h >= 0 && h <= 23) {
            int id = daySchedule[h];
            uint16_t color = getCatColor(id);

            canvas.setTextSize(1);
            canvas.setTextColor((offset == 0) ? WHITE : 0x8410);
            char timeStr[6];
            snprintf(timeStr, sizeof(timeStr), "%02d:00", h);
            canvas.drawCenterString(timeStr, x + boxW / 2, startY - 12);

            if (offset == 0) {
                if (id == 0) {
                    canvas.fillRect(x, startY, boxW, boxH, COL_BG);
                    canvas.drawRect(x, startY, boxW, boxH, WHITE);
                } else {
                    canvas.fillRoundRect(x, startY, boxW, boxH, 3, color);
                    canvas.drawRoundRect(x - 1, startY - 1, boxW + 2, boxH + 2, 4, WHITE);
                }
                
                int triX = x + boxW / 2;
                int triY = startY + boxH + 6;
                canvas.fillTriangle(triX, triY, triX - 4, triY + 4, triX + 4, triY + 4, WHITE);
            } else {
                if (id == 0) {
                    canvas.fillRect(x, startY, boxW, boxH, COL_BG);
                    canvas.drawRect(x, startY, boxW, boxH, 0x4208); 
                } else {
                    canvas.fillRoundRect(x, startY, boxW, boxH, 3, color);
                }
            }
        }
    }

    int currentCat = daySchedule[globalHour];
    canvas.setTextSize(2);
    canvas.setTextColor(currentCat == 0 ? COL_P4 : getCatColor(currentCat));
    canvas.drawCenterString(getCatName(currentCat), 120, 105);
}

void DayTrackApp::drawLegendScreen() {
    canvas.fillScreen(COL_BG);
    canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
    
    canvas.setTextDatum(top_left);
    canvas.setTextSize(1.5);
    canvas.setCursor(5, 5);
    canvas.setTextColor(COL_ACCENT);
    canvas.print("DAY TRACKER [LEGEND]");

    int y = 25;
    canvas.setTextSize(1.0);
    
    canvas.setTextColor(COL_P4); canvas.drawString("MOVE TIME:", 10, y);
    canvas.setTextColor(WHITE);  canvas.drawString("; / .", 85, y);
    
    canvas.setTextColor(COL_P4); canvas.drawString("DATE - / +:", 135, y);
    canvas.setTextColor(WHITE);  canvas.drawString("[ / ]", 200, y);
    y += 12;

    canvas.setTextColor(COL_P4); canvas.drawString("CLEAR:", 10, y);
    canvas.setTextColor(WHITE);  canvas.drawString("Backspace", 85, y);
    y += 18;

    canvas.setTextColor(WHITE);
    canvas.drawString("1: UNI/STUDY", 10, y);     
    canvas.drawString("2: PROJ/PROD", 120, y); y += 14;
    canvas.drawString("3: FAM/FRIEND", 10, y); 
    canvas.drawString("4: CALIS/SPORT", 120, y); y += 14;
    canvas.drawString("5: ROUT/CHORE", 10, y); 
    canvas.drawString("6: READ/RLX/GAME", 120, y); y += 14;
    canvas.drawString("7: WASTE/TRANSIT", 10, y);
    canvas.drawString("8: SLEEP/CLEAR", 120, y); y += 14;

    canvas.setTextColor(COL_ACCENT);
    canvas.drawCenterString("[ Press L to return ]", 120, 120);
}