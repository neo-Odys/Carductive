#include "HabitApp.h"

#include <M5GFX.h>
#include <SD.h>
#include <map>

#include "Global.h"

extern M5Canvas canvas;

void HabitApp::appendHabitState(int cat, const HabitItem& item) {
  File f = SD.open(DATA_PATH "/habits_log.csv", FILE_APPEND);
  if (f) {
    f.printf("%04d-%02d-%02d;%d;%d;%s\n", globalYear, globalMonth, globalDay,
             cat, item.done ? 1 : 0, item.text);
    f.close();
  }
}

void HabitApp::init() {
  if (!SD.exists(DATA_PATH "/habits_master.csv")) {
    // --- MORNING ---
    columns[CAT_MORNING].push_back({"wake up 7:00", false});
    columns[CAT_MORNING].push_back({"calendar check", false});

    // --- AFTERNOON ---
    columns[CAT_AFTERNOON].push_back({"sport", false});
    columns[CAT_AFTERNOON].push_back({"wall stare", false});
    columns[CAT_AFTERNOON].push_back({"study deep focus", false});

    // --- EVENING ---
    columns[CAT_EVENING].push_back({"journal & plan next day", false});
    columns[CAT_EVENING].push_back({"22:00 in bed", false});
    columns[CAT_EVENING].push_back({"read book", false});

    saveMaster();
  } else {
    loadMaster();
  }

  loadDay();
}

bool HabitApp::isTypingMode() { return isTyping; }

bool HabitApp::moveColumnLeft() {
  if (currentColumn > 0) {
    currentColumn--;
    selectedIndex = 0;
    isReordering = false;
    return true;
  }
  return false;
}

bool HabitApp::moveColumnRight() {
  if (currentColumn < CAT_COUNT - 1) {
    currentColumn++;
    selectedIndex = 0;
    isReordering = false;
    return true;
  }
  return false;
}

void HabitApp::setColumn(int col) {
  currentColumn = col;
  selectedIndex = 0;
  isReordering = false;
}

void HabitApp::addHabit(const char* text) {
  if (text[0] == '\0') return;
  HabitItem newItem;
  strncpy(newItem.text, text, 31);
  newItem.text[31] = '\0';
  newItem.done = false;

  if (columns[currentColumn].empty()) {
    columns[currentColumn].push_back(newItem);
    selectedIndex = 0;
  } else {
    columns[currentColumn].insert(
        columns[currentColumn].begin() + selectedIndex + 1, newItem);
    selectedIndex++;
  }

  saveMaster();
}

void HabitApp::deleteHabit() {
  if (columns[currentColumn].empty()) return;
  if (selectedIndex >= 0 &&
      selectedIndex < (int)columns[currentColumn].size()) {
    columns[currentColumn].erase(columns[currentColumn].begin() +
                                 selectedIndex);
    if (selectedIndex >= (int)columns[currentColumn].size() &&
        selectedIndex > 0)
      selectedIndex--;

    saveMaster();
  }
}

void HabitApp::moveHabit(int direction) {
  int newPos = selectedIndex + direction;
  if (newPos >= 0 && newPos < (int)columns[currentColumn].size()) {
    std::swap(columns[currentColumn][selectedIndex],
              columns[currentColumn][newPos]);
    selectedIndex = newPos;
    saveMaster();
  }
}

void HabitApp::changeDate(int delta) {
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
  loadDay();
  selectedIndex = 0;
  isReordering = false;
}

void HabitApp::toggleHabit() {
  if (columns[currentColumn].empty()) return;
  if (selectedIndex >= 0 &&
      selectedIndex < (int)columns[currentColumn].size()) {
    columns[currentColumn][selectedIndex].done =
        !columns[currentColumn][selectedIndex].done;

    appendHabitState(currentColumn, columns[currentColumn][selectedIndex]);

    if (selectedIndex < (int)columns[currentColumn].size() - 1) {
      selectedIndex++;
    }
  }
}

void HabitApp::exportToCSV() {
  std::vector<String> allHabits;
  std::map<String, std::vector<int>> dailyData;

  for (int i = 0; i < CAT_COUNT; i++) {
    for (const auto& item : columns[i]) {
      allHabits.push_back(String(item.text));
    }
  }

  if (SD.exists(DATA_PATH "/habits_log.csv")) {
    File fLog = SD.open(DATA_PATH "/habits_log.csv", FILE_READ);
    if (fLog) {
      while (fLog.available()) {
        String line = fLog.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        int s1 = line.indexOf(';');
        if (s1 == -1) continue;
        String dateStr = line.substring(0, s1);

        int s2 = line.indexOf(';', s1 + 1);
        if (s2 == -1) continue;

        int s3 = line.indexOf(';', s2 + 1);
        if (s3 == -1) continue;

        int done = line.substring(s2 + 1, s3).toInt();
        String habitName = line.substring(s3 + 1);

        int hIdx = -1;
        for (size_t i = 0; i < allHabits.size(); i++) {
          if (allHabits[i] == habitName) {
            hIdx = i;
            break;
          }
        }

        if (hIdx == -1) {
          allHabits.push_back(habitName);
          hIdx = allHabits.size() - 1;
          for (auto& kv : dailyData) {
            kv.second.push_back(0);
          }
        }

        if (dailyData.find(dateStr) == dailyData.end()) {
          dailyData[dateStr] = std::vector<int>(allHabits.size(), 0);
        }
        dailyData[dateStr][hIdx] = done;
      }
      fLog.close();
    }
  }

  File fOut = SD.open(DATA_PATH "/export_habits.csv", FILE_WRITE);
  if (fOut) {
    fOut.print("Date");
    for (const auto& h : allHabits) {
      fOut.print(";");
      fOut.print(h);
    }
    fOut.println();

    for (const auto& kv : dailyData) {
      fOut.print(kv.first);
      for (int val : kv.second) {
        fOut.print(";");
        fOut.print(val);
      }
      fOut.println();
    }
    fOut.close();
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
    } else if (M5Cardputer.Keyboard.isKeyPressed('`') ||
               M5Cardputer.Keyboard.isKeyPressed(KEY_ESC)) {
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

  if (isReordering) {
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
      isReordering = false;
    } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
      moveHabit(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
      moveHabit(1);
    }
    return;
  }

  if (M5Cardputer.Keyboard.isKeyPressed(';')) {
    if (selectedIndex > 0) selectedIndex--;
  } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
    if (selectedIndex < (int)columns[currentColumn].size() - 1) selectedIndex++;
  } else if (M5Cardputer.Keyboard.isKeyPressed('[')) {
    changeDate(-1);
  } else if (M5Cardputer.Keyboard.isKeyPressed(']')) {
    changeDate(1);
  } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
    if (!columns[currentColumn].empty()) isReordering = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed(' ') ||
             M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
    toggleHabit();
  } else if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
    isTyping = true;
    memset(inputBuffer, 0, sizeof(inputBuffer));
  } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
    deleteHabit();
  } else if (M5Cardputer.Keyboard.isKeyPressed('e')) {
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
}

void HabitApp::draw() {
  static int lastLoadedDay = -1, lastLoadedMonth = -1, lastLoadedYear = -1;
  if (lastLoadedDay != globalDay || lastLoadedMonth != globalMonth ||
      lastLoadedYear != globalYear) {
    loadDay();
    lastLoadedDay = globalDay;
    lastLoadedMonth = globalMonth;
    lastLoadedYear = globalYear;
  }

  canvas.fillScreen(COL_BG);

  canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);

  canvas.setTextSize(1.5);
  canvas.setCursor(5, 4);

  if (isReordering) {
    canvas.setTextColor(COL_ACCENT);
    canvas.print("HABITS [REORDER]");
  } else {
    canvas.setTextColor(WHITE);
    canvas.print("HABITS");
  }

  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "[%02d.%02d.%02d]", globalDay, globalMonth,
           globalYear % 100);
  canvas.setTextColor(COL_ACCENT);

  canvas.drawRightString(dateStr, 190, 4);

  int tabY = 20;
  int tabH = 16;
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
    snprintf(buf, 20, "%s (%d)", (i == 0 ? "MORN" : (i == 1 ? "AFT" : "EVE")),
             (int)columns[i].size());

    canvas.drawString(buf, (i * colW) + (colW / 2), tabY + (tabH / 2));
  }
  canvas.setTextDatum(top_left);

  int listStartY = 38;
  int itemH = 19;
  int visibleItems = 5;

  int offset = 0;
  if (selectedIndex >= visibleItems)
    offset = selectedIndex - (visibleItems - 1);

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
        canvas.fillRoundRect(2, y, 236, 17, 3,
                             isReordering ? COL_ACCENT : COL_HIGHLIGHT);
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
      item.done = false;
      columns[cat].push_back(item);
    }
  }
  f.close();
}

void HabitApp::loadDay() {
  for (int i = 0; i < CAT_COUNT; i++) {
    for (auto& item : columns[i]) {
      item.done = false;
    }
  }

  if (!SD.exists(DATA_PATH "/habits_log.csv")) return;

  File f = SD.open(DATA_PATH "/habits_log.csv", FILE_READ);
  if (!f) return;

  char targetDate[16];
  snprintf(targetDate, sizeof(targetDate), "%04d-%02d-%02d", globalYear,
           globalMonth, globalDay);

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int firstSemi = line.indexOf(';');
    if (firstSemi == -1) continue;

    String dateStr = line.substring(0, firstSemi);
    if (dateStr != targetDate) continue;

    int secondSemi = line.indexOf(';', firstSemi + 1);
    int thirdSemi = line.indexOf(';', secondSemi + 1);
    if (secondSemi == -1 || thirdSemi == -1) continue;

    int cat = line.substring(firstSemi + 1, secondSemi).toInt();
    int done = line.substring(secondSemi + 1, thirdSemi).toInt();
    String textStr = line.substring(thirdSemi + 1);

    if (cat >= 0 && cat < CAT_COUNT) {
      for (auto& item : columns[cat]) {
        if (String(item.text) == textStr) {
          item.done = (done == 1);
          break;
        }
      }
    }
  }
  f.close();
}