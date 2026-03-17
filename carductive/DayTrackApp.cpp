#include "DayTrackApp.h"
#include "Global.h"
#include <M5GFX.h>
#include <SD.h>

extern M5Canvas canvas;

#define CAT_UNI          0x64BD
#define CAT_STUDY        0x4416
#define CAT_PROD         0x3666
#define CAT_WORK         0x3D8E
#define CAT_FAM          0xFC60
#define CAT_FRIENDS      0xF52C
#define CAT_CALIS        0x939B
#define CAT_RUNNING      0x895C
#define CAT_SPORT        0xBABA
#define CAT_ROUTINE      0xA4B0
#define CAT_CHORES       0x6B2B
#define CAT_TRANSIT      0x8A22
#define CAT_READ         0x05FF
#define CAT_GAMES        0x2595
#define CAT_VACATION     0xFEA0
#define CAT_WASTE        0xD8A7
#define CAT_PROJ         0xF8B2
#define CAT_EMBEDDED     0xB18F
#define CAT_BUSINESS     0xCAEB
#define CAT_SLEEP        0x8410

#define CAT_EMPTY        0x0000

static bool isDirty = false;
static unsigned long lastEditTime = 0;

static int getDayOfYear(int day, int month, int year) {
    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) daysInMonth[1] = 29;
    int doy = 0;
    for (int i = 0; i < month - 1; i++) {
        doy += daysInMonth[i];
    }
    return doy + day;
}

static void exportToCSV() {
    char binPath[64], csvPath[64];
    snprintf(binPath, sizeof(binPath), DATA_PATH "/track_%04d.bin", globalYear);
    snprintf(csvPath, sizeof(csvPath), DATA_PATH "/export_%04d.csv", globalYear);
    
    if (SD.exists(binPath)) {
        File fIn = SD.open(binPath, FILE_READ);
        File fOut = SD.open(csvPath, FILE_WRITE);
        if (fIn && fOut) {
            fOut.println("DayOfYear;H00;H01;H02;H03;H04;H05;H06;H07;H08;H09;H10;H11;H12;H13;H14;H15;H16;H17;H18;H19;H20;H21;H22;H23");
            uint8_t buffer[24];
            int dayCounter = 1;
            while (fIn.available()) {
                int bytesRead = fIn.read(buffer, 24);
                if (bytesRead == 24) {
                    bool hasData = false;
                    for (int i = 0; i < 24; i++) {
                        if (buffer[i] != 0) hasData = true;
                    }
                    if (hasData) {
                        fOut.printf("%d;", dayCounter);
                        for (int i = 0; i < 24; i++) {
                            fOut.printf("%d%s", buffer[i], (i == 23) ? "" : ";");
                        }
                        fOut.println();
                    }
                }
                dayCounter++;
            }
            fIn.close();
            fOut.close();
        }
    }
}

void DayTrackApp::init() {
    loadForCurrentDate();
}

uint16_t DayTrackApp::getCatColor(int id) {
    switch(id) {
        case 1: return CAT_UNI;         case 2: return CAT_STUDY;
        case 3: return CAT_PROD;        case 4: return CAT_WORK;
        case 5: return CAT_FAM;         case 6: return CAT_FRIENDS;
        case 7: return CAT_CALIS;       case 8: return CAT_RUNNING;   case 9: return CAT_SPORT;
        case 10: return CAT_ROUTINE;    case 11: return CAT_CHORES;   case 12: return CAT_TRANSIT;
        case 13: return CAT_READ;       case 14: return CAT_GAMES;    case 15: return CAT_VACATION;
        case 16: return CAT_WASTE;
        case 17: return CAT_PROJ;       case 18: return CAT_EMBEDDED; case 19: return CAT_BUSINESS;
        case 20: return CAT_SLEEP;
        default: return CAT_EMPTY;
    }
}

const char* DayTrackApp::getCatName(int id) {
    switch(id) {
        case 1: return "UNIVERSITY";    case 2: return "STUDY";
        case 3: return "PRODUCTIVE";    case 4: return "WORK";
        case 5: return "FAMILY";        case 6: return "FRIENDS";
        case 7: return "CALISTHENICS";  case 8: return "RUNNING";     case 9: return "SPORT";
        case 10: return "ROUTINE";      case 11: return "CHORES";     case 12: return "TRANSIT";
        case 13: return "READ";         case 14: return "GAMES";      case 15: return "VACATION";
        case 16: return "WASTE";
        case 17: return "PROJECTS";     case 18: return "EMBEDDED";   case 19: return "BUSINESS";
        case 20: return "SLEEP";
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
    
    for(int i = 0; i <= 6; i++) daySchedule[i] = 20;
    daySchedule[22] = 20; 
    daySchedule[23] = 20;
    isDirty = false;

    char path[64];
    snprintf(path, sizeof(path), DATA_PATH "/track_%04d.bin", globalYear);
    
    if (SD.exists(path)) {
        File f = SD.open(path, FILE_READ);
        if (f) {
            uint32_t offset = (getDayOfYear(globalDay, globalMonth, globalYear) - 1) * 24;
            f.seek(offset);
            uint8_t buffer[24];
            int bytesRead = f.read(buffer, 24);
            if (bytesRead == 24) {
                for (int i = 0; i < 24; i++) {
                    daySchedule[i] = buffer[i];
                }
            }
            f.close();
        }
    }
}
void DayTrackApp::saveForCurrentDate() {
    char path[64];
    snprintf(path, sizeof(path), DATA_PATH "/track_%04d.bin", globalYear);
    
    File f;
    if (!SD.exists(path)) {
        f = SD.open(path, FILE_WRITE);
        if (f) {
            uint8_t defaultDay[24] = {0};
            
            for(int i = 0; i <= 6; i++) defaultDay[i] = 20; 
            defaultDay[22] = 20;
            defaultDay[23] = 20;

            for (int i = 0; i < 366; i++) {
                f.write(defaultDay, 24);
            }
            f.close();
        }
    }
    
    f = SD.open(path, "r+");
    if (!f) return;

    uint32_t offset = (getDayOfYear(globalDay, globalMonth, globalYear) - 1) * 24;
    f.seek(offset);
    
    uint8_t buffer[24];
    for (int i = 0; i < 24; i++) {
        buffer[i] = (uint8_t)daySchedule[i];
    }
    
    f.write(buffer, 24);
    f.close();
    isDirty = false;
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

    bool stateChanged = false;

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
        if (isDirty) saveForCurrentDate();
        changeDate(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
        if (isDirty) saveForCurrentDate();
        changeDate(1);
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('1')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 1) ? 2 : 1;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('2')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 3) ? 4 : 3;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('3')) {
        int c = daySchedule[globalHour];
        daySchedule[globalHour] = (c == 5) ? 6 : 5;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('4')) {
        int c = daySchedule[globalHour];
        if (c == 7) daySchedule[globalHour] = 8;
        else if (c == 8) daySchedule[globalHour] = 9;
        else daySchedule[globalHour] = 7;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('5')) {
        int c = daySchedule[globalHour];
        if (c == 10) daySchedule[globalHour] = 11;
        else if (c == 11) daySchedule[globalHour] = 12;
        else daySchedule[globalHour] = 10;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('6')) {
        int c = daySchedule[globalHour];
        if (c == 13) daySchedule[globalHour] = 14;
        else if (c == 14) daySchedule[globalHour] = 15;
        else daySchedule[globalHour] = 13;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('7')) {
        daySchedule[globalHour] = 16;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('8')) {
        int c = daySchedule[globalHour];
        if (c == 17) daySchedule[globalHour] = 18;
        else if (c == 18) daySchedule[globalHour] = 19;
        else daySchedule[globalHour] = 17;
        stateChanged = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('9')) {
        daySchedule[globalHour] = 20;
        stateChanged = true;
    }
    else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
        daySchedule[globalHour] = 0;
        stateChanged = true;
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('l')) {
        showLegend = true;
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('e')) {
        exportToCSV();
        
        canvas.fillScreen(COL_BG);
        canvas.setTextDatum(middle_center);
        canvas.setTextColor(COL_ACCENT);
        canvas.setTextSize(2);
        canvas.drawString("EXPORTED TO CSV", 120, 60);
        canvas.pushSprite(0, 0);
        delay(1000);
        canvas.setTextDatum(top_left);
    }

    if (stateChanged) {
        isDirty = true;
        lastEditTime = millis();
    }

    if (isDirty && millis() - lastEditTime > 2000) {
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

    // --- NOWY KOD: Liczniki STUDY i EMBEDDED ---
    int studyCount = 0;
    int embedCount = 0;
    for (int i = 0; i < 24; i++) {
        if (daySchedule[i] == 2) studyCount++;    // ID 2 to STUDY
        if (daySchedule[i] == 18) embedCount++;   // ID 18 to EMBEDDED
    }

    canvas.setTextSize(1);
    
    // Lewy dolny róg: STUDY
    canvas.setTextDatum(bottom_left);
    canvas.setTextColor(CAT_STUDY); 
    char studyStr[16];
    snprintf(studyStr, sizeof(studyStr), "%d/3 STUDY", studyCount);
    canvas.drawString(studyStr, 5, 130);

    // Prawy dolny róg: EMBEDDED
    canvas.setTextDatum(bottom_right);
    canvas.setTextColor(CAT_EMBEDDED);
    char embedStr[16];
    snprintf(embedStr, sizeof(embedStr), "%d/2 EMBED", embedCount);
    canvas.drawString(embedStr, 235, 130);

    // Przywrócenie domyślnego punktu odniesienia rysowania
    canvas.setTextDatum(top_left);
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
    
    canvas.setTextColor(COL_P4); canvas.drawString("MOVE TIME:", 5, y);
    canvas.setTextColor(WHITE);  canvas.drawString("; / .", 85, y);
    
    canvas.setTextColor(COL_P4); canvas.drawString("DATE - / +:", 135, y);
    canvas.setTextColor(WHITE);  canvas.drawString("[ / ]", 200, y);
    y += 14;

    canvas.setTextColor(COL_P4); canvas.drawString("EXPORT:", 5, y);
    canvas.setTextColor(WHITE);  canvas.drawString("e", 85, y);
    canvas.setTextColor(COL_P4); canvas.drawString("CLEAR:", 135, y);
    canvas.setTextColor(WHITE);  canvas.drawString("Backspace", 185, y);
    y += 16;

    canvas.setTextColor(WHITE);
    canvas.drawString("1(Blu): UNI/STDY", 5, y);     
    canvas.drawString("2(Grn): PROD/WRK", 125, y); y += 12;
    canvas.drawString("3(Orn): FAM/FRND", 5, y); 
    canvas.drawString("4(Pur): CAL/RUN/SPT", 125, y); y += 12;
    canvas.drawString("5(Brn): ROU/CHO/TRN", 5, y); 
    canvas.drawString("6(Cyn): READ/GAM/VAC", 125, y); y += 12;
    canvas.drawString("7(Red): WASTE", 5, y);
    canvas.drawString("8(Pnk): PRJ/EMB/BUS", 125, y); y += 12;
    canvas.drawString("9(Gry): SLEEP", 5, y);

    canvas.setTextColor(COL_ACCENT);
    canvas.drawCenterString("[ Press L to return ]", 120, 126);
}