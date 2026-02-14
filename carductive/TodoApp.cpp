#include "TodoApp.h"
#include "PomodoroApp.h"
#include <M5GFX.h>
#include <SD.h>
#include <algorithm>
#include "Global.h"

const char* PRIO_LABELS[4] = {
    "# ",
    "$ ",
    "* ",
    "| "
};
extern M5Canvas canvas;
extern PomodoroApp pomodoroApp;
extern AppMode currentMode;

void TodoApp::init() { loadFromSD(); }

bool TodoApp::isTypingMode() { return isTyping; }

void TodoApp::adjustVal(int& field, int delta, int minV, int maxV) {
  if (selectedIndex >= 0 && selectedIndex < (int)todoList.size()) {
    if (strcmp(todoList[selectedIndex].text, SEP_TAG) != 0) {
      field += delta;
      if (field < minV) field = minV;
      if (field > maxV) field = maxV;
      saveToSD();
    }
  }
}

void TodoApp::update() {
  if (showLegend) {
    if (M5Cardputer.Keyboard.isKeyPressed('l') ||
        M5Cardputer.Keyboard.isKeyPressed('`')) {
      showLegend = false;
    }
    return;
  }

  if (isTyping) {
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
      if (inputBuffer[0] != '\0') addTask(inputBuffer);
      memset(inputBuffer, 0, sizeof(inputBuffer));
      isTyping = false;
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
      int len = strlen(inputBuffer);
      if (len > 0) inputBuffer[len - 1] = '\0';
    } else if (M5Cardputer.Keyboard.isKeyPressed('`')) {
      isTyping = false;
      memset(inputBuffer, 0, sizeof(inputBuffer));
    } else {
      for (auto c : M5Cardputer.Keyboard.keysState().word) {
        int len = strlen(inputBuffer);
        if (len < 18 && len < (int)sizeof(inputBuffer) - 1) {
          inputBuffer[len] = c;
          inputBuffer[len + 1] = '\0';
        }
      }
    }
    return;
  }

  if (isReordering) {
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
      isReordering = false;
      return;
    }
    if (M5Cardputer.Keyboard.isKeyPressed(';')) {
      moveTask(-1);
      if (selectedIndex > 0) selectedIndex--;
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
      moveTask(1);
      if (selectedIndex < (int)todoList.size() - 1) selectedIndex++;
    }
    return;
  }

  if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
    isTyping = true;
    memset(inputBuffer, 0, sizeof(inputBuffer));
  } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
    if (!todoList.empty()) isReordering = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed(' ') ||
             M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
    toggleDone();
  } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
    deleteTask();
  } else if (M5Cardputer.Keyboard.isKeyPressed('l')) {
    showLegend = true;
  } else if (M5Cardputer.Keyboard.isKeyPressed('c')) {
    removeDoneTasks();
  } else if (M5Cardputer.Keyboard.isKeyPressed('p')) {
    if (selectedIndex >= 0 && selectedIndex < (int)todoList.size()) {
        pomodoroApp.setTask(todoList[selectedIndex].text);
        currentMode = APP_POMODORO;
    }
  } else if (M5Cardputer.Keyboard.isKeyPressed('q'))
    adjustVal(todoList[selectedIndex].priority, -1, 1, 4);
  else if (M5Cardputer.Keyboard.isKeyPressed('w'))
    adjustVal(todoList[selectedIndex].priority, 1, 1, 4);
  else if (M5Cardputer.Keyboard.isKeyPressed('['))
    adjustVal(todoList[selectedIndex].urgency, -1, 0, 3);
  else if (M5Cardputer.Keyboard.isKeyPressed(']'))
    adjustVal(todoList[selectedIndex].urgency, 1, 0, 3);
  else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
    if (selectedIndex > 0) selectedIndex--;
  } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
    if (selectedIndex < (int)todoList.size() - 1) selectedIndex++;
  }
}

void TodoApp::draw() {
  if (showLegend) {
    drawLegendScreen();
    return;
  }

  int listSize = todoList.size();

  canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
  canvas.setTextSize(1.5);
  canvas.setCursor(5, 5);

  if (isReordering) {
    canvas.setTextColor(COL_ACCENT);
    canvas.print("TODO [REORDER]");
  } else {
    canvas.setTextColor(COL_TEXT_NORM);
    canvas.printf("TODO [%d]", listSize > 0 ? listSize - 1 : 0);
  }

  int visibleItems = 6;
  int startY = 22;

  static int scrollOffset = 0;

  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  }
  if (selectedIndex >= scrollOffset + visibleItems) {
    scrollOffset = selectedIndex - visibleItems + 1;
  }
  
  if (scrollOffset > listSize - visibleItems) scrollOffset = listSize - visibleItems;
  if (scrollOffset < 0) scrollOffset = 0;
  
  int viewOffset = scrollOffset;

  for (int i = 0; i < visibleItems; i++) {
    int idx = viewOffset + i;
    if (idx >= listSize) break;

    int y = startY + (i * 19);
    TodoItem& item = todoList[idx];
    bool isSel = (idx == selectedIndex);

    if (isSel)
      canvas.fillRoundRect(2, y, 236, 17, 3,
                           isReordering ? COL_ACCENT : COL_HIGHLIGHT);

    if (strcmp(item.text, SEP_TAG) == 0) {
      canvas.drawFastHLine(10, y + 8, 220, COL_SEPARATOR);
      if (isSel) canvas.drawFastHLine(10, y + 9, 220, COL_SEPARATOR);
      continue;
    }

    uint16_t prioCol =
        (item.done) ? COL_TEXT_DONE
                    : (item.priority == 1
                           ? COL_P1
                           : (item.priority == 2
                                  ? COL_P2
                                  : (item.priority == 3 ? COL_P3 : COL_P4)));
    uint16_t textCol = item.done ? COL_TEXT_DONE : COL_TEXT_NORM;

    const char* prefixBuf = PRIO_LABELS[item.priority - 1];

    int prefixW = canvas.textWidth(prefixBuf);
    int maxLen = 30 - strlen(prefixBuf);

    char content[64];
    int textLen = strlen(item.text);
    
    if (textLen > maxLen) {
      strncpy(content, item.text, maxLen - 2);
      content[maxLen - 2] = '\0';
      strcat(content, "..");
    } else {
      strcpy(content, item.text);
    }

    canvas.setTextColor(prioCol);
    canvas.drawString(prefixBuf, 8, y + 3);

    canvas.setTextColor(textCol);
    canvas.drawString(content, 8 + prefixW, y + 3);

    if (item.priority == 1 && !item.done) {
      canvas.setTextColor(prioCol);
      canvas.drawString(prefixBuf, 9, y + 3);
      canvas.setTextColor(textCol);
      canvas.drawString(content, 9 + prefixW, y + 3);
    }

    if (item.done) {
      int totalW = prefixW + canvas.textWidth(content);
      canvas.drawFastHLine(8, y + 8, totalW, COL_TEXT_DONE);
    }

    if (item.urgency > 0) {
      for (int d = 0; d < item.urgency; d++) {
        uint16_t dotCol;

        if (item.done) {
          dotCol = COL_TEXT_DONE;
        } else {
          dotCol = COL_ACCENT;
        }

        canvas.fillCircle(225 - (d * 8), y + 8, 2, dotCol);
      }
    }
  }

  if (isTyping) {
    canvas.fillRect(0, 115, 240, 20, COL_HEADER_BG);
    canvas.setCursor(5, 118);
    canvas.setTextColor(COL_ACCENT);
    canvas.printf("> %s_", inputBuffer);
  }
}

void TodoApp::drawLegendScreen() {
  canvas.fillScreen(COL_BG);

  canvas.fillRect(0, 0, 240, 20, COL_HEADER_BG);
  canvas.setTextDatum(top_left);

  canvas.setTextSize(1.5);
  canvas.setCursor(5, 5);
  canvas.setTextColor(COL_ACCENT);
  canvas.print("TODO [LEGEND]");

  canvas.setTextSize(1.0);
  int y = 30;
  int x1 = 10;
  int x2 = 160;
  int lh = 12;

  canvas.setTextColor(COL_P4);
  canvas.drawString("NAVIGATE", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("; / .", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("NEW TASK", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("\\", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("DONE / UNDONE", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("Space", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("POMODORO", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("p", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("PRIORITY", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("q / w", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("URGENCY", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("[ / ]", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("REORDER", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("Enter", x2, y);
  y += lh;

  canvas.setTextColor(COL_P4);
  canvas.drawString("CLEAR DONE", x1, y);
  canvas.setTextColor(WHITE);
  canvas.drawString("c", x2, y);
  y += lh;

  canvas.setTextColor(COL_ACCENT);
  canvas.drawCenterString("[ Press ESC or L to return ]", 120, 120);
}

void TodoApp::saveToSD() {
  File f = SD.open(DATA_PATH "/todo.bin", FILE_WRITE);
  if (f) {
    for (const auto& item : todoList) {
      f.write((const uint8_t*)&item, sizeof(TodoItem));
    }
    f.close();
  }
}

void TodoApp::loadFromSD() {
  todoList.clear();

  if (SD.exists(DATA_PATH "/todo.bin")) {
    File f = SD.open(DATA_PATH "/todo.bin", FILE_READ);
    if (f) {
      TodoItem tempItem;
      while (f.read((uint8_t*)&tempItem, sizeof(TodoItem)) ==
             sizeof(TodoItem)) {
        todoList.push_back(tempItem);
      }
      f.close();
    }
  }

  if (todoList.empty()) {
    addTask("Type --- for line");
    addTask("---");

    TodoItem t3;
    memset(&t3, 0, sizeof(TodoItem));
    strncpy(t3.text, "Priority: q / w", 19);
    t3.priority = 2;
    t3.urgency = 0;
    t3.done = false;
    todoList.push_back(t3);

    TodoItem t4;
    memset(&t4, 0, sizeof(TodoItem));
    strncpy(t4.text, "Urgency: [ / ]", 19);
    t4.priority = 4;
    t4.urgency = 2;
    t4.done = false;
    todoList.push_back(t4);

    addTask("New: \\ , Done: Spc");

    addTask("Move: Enter");

    addTask("Press L for help");

    saveToSD();
  }
}

void TodoApp::addTask(const char* text) {
  if (text == nullptr || text[0] == '\0') return;
  TodoItem newItem;
  strncpy(newItem.text, text, sizeof(newItem.text) - 1);
  newItem.text[sizeof(newItem.text) - 1] = '\0';
  newItem.priority = 4;
  newItem.urgency = 0;
  newItem.done = false;

  if (todoList.empty()) {
    todoList.push_back(newItem);
    selectedIndex = 0;
  } else {
    todoList.insert(todoList.begin() + selectedIndex + 1, newItem);
    selectedIndex++;
  }

  saveToSD();
}

void TodoApp::deleteTask() {
  if (selectedIndex >= 0 && selectedIndex < (int)todoList.size()) {
    todoList.erase(todoList.begin() + selectedIndex);
    if (selectedIndex >= (int)todoList.size() && selectedIndex > 0)
      selectedIndex--;
    saveToSD();
  }
}

void TodoApp::removeDoneTasks() {
  auto it = todoList.begin();
  bool changed = false;
  while (it != todoList.end()) {
    if (it->done) {
      it = todoList.erase(it);
      changed = true;
    } else {
      ++it;
    }
  }
  if (changed) {
    if (selectedIndex >= (int)todoList.size()) {
      selectedIndex = (int)todoList.size() > 0 ? (int)todoList.size() - 1 : 0;
    }
    saveToSD();
  }
}

void TodoApp::toggleDone() {
  if (selectedIndex >= 0 && selectedIndex < (int)todoList.size()) {
    if (strcmp(todoList[selectedIndex].text, SEP_TAG) == 0) return;

    todoList[selectedIndex].done = !todoList[selectedIndex].done;

    if (selectedIndex < (int)todoList.size() - 1) {
      selectedIndex++;
    }

    saveToSD();
  }
}

void TodoApp::moveTask(int direction) {
  int n = selectedIndex + direction;
  if (n >= 0 && n < (int)todoList.size()) {
    std::swap(todoList[selectedIndex], todoList[n]);
    saveToSD();
  }
}