#include "DayTrackApp.h"

#include <M5GFX.h>
#include <SD.h>

#include "Global.h"

extern M5Canvas canvas;

#define CAT_UNI 0x64BD
#define CAT_STUDY 0x4416
#define CAT_PROD 0x3666
#define CAT_WORK 0x3D8E
#define CAT_FAM 0xFC60
#define CAT_FRIENDS 0xF52C
#define CAT_CALIS 0x939B
#define CAT_RUNNING 0x895C
#define CAT_SPORT 0xBABA
#define CAT_ROUTINE 0xA4B0
#define CAT_CHORES 0x6B2B
#define CAT_TRANSIT 0x8A22
#define CAT_READ 0x05FF
#define CAT_GAMES 0x2595
#define CAT_VACATION 0xFEA0
#define CAT_WASTE 0xD8A7
#define CAT_PROJ 0xF8B2
#define CAT_EMBEDDED 0xB18F
#define CAT_BUSINESS 0xCAEB
#define CAT_SLEEP 0x8410
#define CAT_EMPTY 0x0000

static bool isDirty = false;
static unsigned long lastEditTime = 0;

static int getDayOfYear(int day, int month, int year) {
  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
    daysInMonth[1] = 29;
  int doy = 0;
  for (int i = 0; i < month - 1; i++) doy += daysInMonth[i];
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
      // Zmieniony nagłówek pliku CSV na format 00:00, 01:00 itd.
      fOut.println(
          "Date;00:00;01:00;02:00;03:00;04:00;05:00;06:00;07:00;08:00;09:00;10:"
          "00;11:00;12:00;13:00;"
          "14:00;15:00;16:00;17:00;18:00;19:00;20:00;21:00;22:00;23:00");

      uint8_t buffer[24];
      int dayCounter = 1;

      int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if (globalYear % 4 == 0 &&
          (globalYear % 100 != 0 || globalYear % 400 == 0)) {
        daysInMonth[1] = 29;
      }

      while (fIn.available()) {
        if (fIn.read(buffer, 24) == 24) {
          bool hasData = false;
          for (int i = 0; i < 24; i++) {
            if (buffer[i] != 0) hasData = true;
          }
          if (hasData) {
            int month = 1;
            int day = dayCounter;
            for (int i = 0; i < 12; i++) {
              if (day > daysInMonth[i]) {
                day -= daysInMonth[i];
                month++;
              } else {
                break;
              }
            }

            fOut.printf("%04d-%02d-%02d;", globalYear, month, day);

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
void DayTrackApp::init() { loadForCurrentDate(); }

uint16_t DayTrackApp::getCatColor(int id) {
  switch (id) {
    case 1:
      return CAT_UNI;
    case 2:
      return CAT_STUDY;
    case 3:
      return CAT_PROD;
    case 4:
      return CAT_WORK;
    case 5:
      return CAT_FAM;
    case 6:
      return CAT_FRIENDS;
    case 7:
      return CAT_CALIS;
    case 8:
      return CAT_RUNNING;
    case 9:
      return CAT_SPORT;
    case 10:
      return CAT_ROUTINE;
    case 11:
      return CAT_CHORES;
    case 12:
      return CAT_TRANSIT;
    case 13:
      return CAT_READ;
    case 14:
      return CAT_GAMES;
    case 15:
      return CAT_VACATION;
    case 16:
      return CAT_WASTE;
    case 17:
      return CAT_PROJ;
    case 18:
      return CAT_EMBEDDED;
    case 19:
      return CAT_BUSINESS;
    case 20:
      return CAT_SLEEP;
    default:
      return CAT_EMPTY;
  }
}

const char* DayTrackApp::getCatName(int id) {
  switch (id) {
    case 1:
      return "UNIVERSITY";
    case 2:
      return "STUDY";
    case 3:
      return "PRODUCTIVE";
    case 4:
      return "WORK";
    case 5:
      return "FAMILY";
    case 6:
      return "FRIENDS";
    case 7:
      return "CALISTHENICS";
    case 8:
      return "RUNNING";
    case 9:
      return "SPORT";
    case 10:
      return "ROUTINE";
    case 11:
      return "CHORES";
    case 12:
      return "TRANSIT";
    case 13:
      return "READ";
    case 14:
      return "GAMES";
    case 15:
      return "VACATION";
    case 16:
      return "WASTE";
    case 17:
      return "PROJECTS";
    case 18:
      return "EMBEDDED";
    case 19:
      return "BUSINESS";
    case 20:
      return "SLEEP";
    default:
      return "EMPTY";
  }
}

void DayTrackApp::changeDate(int delta) {
  globalDay += delta;
  int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (globalYear % 4 == 0 && (globalYear % 100 != 0 || globalYear % 400 == 0))
    daysInMonth[2] = 29;

  if (globalDay > daysInMonth[globalMonth]) {
    globalDay = 1;
    globalMonth++;
    if (globalMonth > 12) {
      globalMonth = 1;
      globalYear++;
    }
  } else if (globalDay < 1) {
    globalMonth--;
    if (globalMonth < 1) {
      globalMonth = 12;
      globalYear--;
    }
    globalDay = daysInMonth[globalMonth];
  }
  saveGlobalDate();
  loadForCurrentDate();
}

void DayTrackApp::loadForCurrentDate() {
  for (int i = 0; i < 24; i++) daySchedule[i] = 0;
  for (int i = 0; i <= 6; i++) daySchedule[i] = 20;
  daySchedule[22] = 20;
  daySchedule[23] = 20;
  isDirty = false;

  char path[64];
  snprintf(path, sizeof(path), DATA_PATH "/track_%04d.bin", globalYear);
  if (SD.exists(path)) {
    File f = SD.open(path, FILE_READ);
    if (f) {
      uint32_t offset =
          (getDayOfYear(globalDay, globalMonth, globalYear) - 1) * 24;
      f.seek(offset);
      uint8_t buffer[24];
      if (f.read(buffer, 24) == 24) {
        for (int i = 0; i < 24; i++) daySchedule[i] = buffer[i];
      }
      f.close();
    }
  }
}

void DayTrackApp::saveForCurrentDate() {
  char path[64];
  snprintf(path, sizeof(path), DATA_PATH "/track_%04d.bin", globalYear);
  if (!SD.exists(path)) {
    File f = SD.open(path, FILE_WRITE);
    if (f) {
      uint8_t def[24] = {0};
      for (int i = 0; i <= 6; i++) def[i] = 20;
      def[22] = 20;
      def[23] = 20;
      for (int i = 0; i < 366; i++) f.write(def, 24);
      f.close();
    }
  }
  File f = SD.open(path, "r+");
  if (!f) return;
  uint32_t offset = (getDayOfYear(globalDay, globalMonth, globalYear) - 1) * 24;
  f.seek(offset);
  f.write(daySchedule, 24);
  f.close();
  isDirty = false;
}

void DayTrackApp::update() {
  if (showLegend) {
    if (M5Cardputer.Keyboard.isKeyPressed('l') ||
        M5Cardputer.Keyboard.isKeyPressed('`') ||
        M5Cardputer.Keyboard.isKeyPressed(KEY_ESC))
      showLegend = false;
    return;
  }

  bool stateChanged = false;

  // Przewijanie godzin z automatyczną zmianą daty
  if (M5Cardputer.Keyboard.isKeyPressed(';')) {
    if (globalHour < 23) {
      globalHour++;
    } else {
      if (isDirty) saveForCurrentDate();
      changeDate(1);
      globalHour = 0;
    }
    saveGlobalDate();
  } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
    if (globalHour > 0) {
      globalHour--;
    } else {
      if (isDirty) saveForCurrentDate();
      changeDate(-1);
      globalHour = 23;
    }
    saveGlobalDate();
  } else if (M5Cardputer.Keyboard.isKeyPressed('[')) {
    if (isDirty) saveForCurrentDate();
    changeDate(-1);
  } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
    if (isDirty) saveForCurrentDate();
    changeDate(1);
  }
  // Obsługa kategorii
  else if (M5Cardputer.Keyboard.isKeyPressed('1')) {
    daySchedule[globalHour] = (daySchedule[globalHour] == 1) ? 2 : 1;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('2')) {
    daySchedule[globalHour] = (daySchedule[globalHour] == 3) ? 4 : 3;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('3')) {
    daySchedule[globalHour] = (daySchedule[globalHour] == 5) ? 6 : 5;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('4')) {
    int c = daySchedule[globalHour];
    daySchedule[globalHour] = (c == 7) ? 8 : (c == 8 ? 9 : 7);
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('5')) {
    int c = daySchedule[globalHour];
    daySchedule[globalHour] = (c == 10) ? 11 : (c == 11 ? 12 : 10);
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('6')) {
    int c = daySchedule[globalHour];
    daySchedule[globalHour] = (c == 13) ? 14 : (c == 14 ? 15 : 13);
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('7')) {
    daySchedule[globalHour] = 16;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('8')) {
    int c = daySchedule[globalHour];
    daySchedule[globalHour] = (c == 17) ? 18 : (c == 18 ? 19 : 17);
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('9')) {
    daySchedule[globalHour] = 20;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
    daySchedule[globalHour] = 0;
    stateChanged = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('l'))
    showLegend = true;
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
  if (isDirty && millis() - lastEditTime > 2000) saveForCurrentDate();
}

void DayTrackApp::draw() {
  // --- KLUCZOWA NAPRAWA: Synchronizacja daty ---
  static int lastLD = -1, lastLM = -1, lastLY = -1;
  if (lastLD != globalDay || lastLM != globalMonth || lastLY != globalYear) {
    loadForCurrentDate();
    lastLD = globalDay;
    lastLM = globalMonth;
    lastLY = globalYear;
  }

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
  snprintf(dateStr, sizeof(dateStr), "[%02d.%02d.%02d]", globalDay, globalMonth,
           globalYear % 100);
  canvas.setTextColor(COL_ACCENT);
  canvas.drawRightString(dateStr, 190, 4);

  drawTimeline();
}

void DayTrackApp::drawTimeline() {
  int boxW = 34, boxH = 40, startY = 48, gap = 8, centerX = 120;

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
        canvas.fillTriangle(x + boxW / 2, startY + boxH + 6, x + boxW / 2 - 4,
                            startY + boxH + 10, x + boxW / 2 + 4,
                            startY + boxH + 10, WHITE);
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

  // Cele dnia 1/1
  int studyCount = 0, embedCount = 0;
  for (int i = 0; i < 24; i++) {
    if (daySchedule[i] == 2) studyCount++;
    if (daySchedule[i] == 18) embedCount++;
  }

  canvas.setTextSize(1);
  canvas.setTextDatum(bottom_left);
  canvas.setTextColor(CAT_STUDY);
  char studyStr[16];
  snprintf(studyStr, sizeof(studyStr), "%d/1 STUDY", studyCount);
  canvas.drawString(studyStr, 5, 130);

  canvas.setTextDatum(bottom_right);
  canvas.setTextColor(CAT_EMBEDDED);
  char embedStr[16];
  snprintf(embedStr, sizeof(embedStr), "%d/1 EMBED", embedCount);
  canvas.drawString(embedStr, 235, 130);

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
  canvas.setTextColor(COL_P4);
  canvas.drawString("MOVE TIME:", 5, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("; / .", 85, y);
  canvas.setTextColor(COL_P4);
  canvas.drawString("DATE - / +:", 135, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("[ / ]", 200, y);
  y += 14;
  canvas.setTextColor(COL_P4);
  canvas.drawString("EXPORT:", 5, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("e", 85, y);
  canvas.setTextColor(COL_P4);
  canvas.drawString("CLEAR:", 135, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("Backspace", 185, y);
  y += 16;
  canvas.setTextColor(WHITE);
  canvas.drawString("1: UNI/STDY", 5, y);
  canvas.drawString("2: PROD/WRK", 125, y);
  y += 12;
  canvas.drawString("3: FAM/FRND", 5, y);
  canvas.drawString("4: CAL/RUN/SPT", 125, y);
  y += 12;
  canvas.drawString("5: ROU/CHO/TRN", 5, y);
  canvas.drawString("6: READ/GAM/VAC", 125, y);
  y += 12;
  canvas.drawString("7: WASTE", 5, y);
  canvas.drawString("8: PRJ/EMB/BUS", 125, y);
  y += 12;
  canvas.drawString("9: SLEEP", 5, y);
  canvas.setTextColor(COL_ACCENT);
  canvas.drawCenterString("[ Press L to return ]", 120, 126);
}