// ==================== carductive.ino ====================
#include <M5Cardputer.h>
#include <M5GFX.h>
#include <SD.h>

#include "DayTrackApp.h"
#include "Global.h"
#include "HabitApp.h"
#include "PomodoroApp.h"
#include "TodoApp.h"

M5Canvas canvas(&M5Cardputer.Display);

TodoApp todoApp;
PomodoroApp pomodoroApp;
HabitApp habitApp;
DayTrackApp dayTrackApp;

AppMode currentMode = APP_TODO;

unsigned long lastActivityTime = 0;
bool isScreenDimmed = false;
bool needsRedraw = true;

int globalDay = 26;
int globalMonth = 2;
int globalYear = 2026;
int globalHour = 12;

void saveGlobalDate() {
    File sf = SD.open(DATA_PATH "/shared_date.bin", FILE_WRITE);
    if (sf) {
        sf.write((uint8_t*)&globalYear, sizeof(int));
        sf.write((uint8_t*)&globalMonth, sizeof(int));
        sf.write((uint8_t*)&globalDay, sizeof(int));
        sf.write((uint8_t*)&globalHour, sizeof(int));
        sf.close();
    }
}

void loadGlobalDate() {
    if (SD.exists(DATA_PATH "/shared_date.bin")) {
        File sf = SD.open(DATA_PATH "/shared_date.bin", FILE_READ);
        if (sf) {
            sf.read((uint8_t*)&globalYear, sizeof(int));
            sf.read((uint8_t*)&globalMonth, sizeof(int));
            sf.read((uint8_t*)&globalDay, sizeof(int));
            sf.read((uint8_t*)&globalHour, sizeof(int));
            sf.close();
        }
    }
}

void handleUpdate() {
  switch (currentMode) {
    case APP_TODO:
      todoApp.update();
      break;
    case APP_POMODORO:
      pomodoroApp.update();
      break;
    case APP_HABIT:
      habitApp.update();
      break;
    case APP_DAY:
      dayTrackApp.update();
      break;
  }
}

void handleDraw() {
  switch (currentMode) {
    case APP_TODO:
      todoApp.draw();
      break;
    case APP_POMODORO:
      pomodoroApp.draw();
      break;
    case APP_HABIT:
      habitApp.draw();
      break;
    case APP_DAY:
      dayTrackApp.draw();
      break;
  }
}

void playIntro() {
  canvas.setTextDatum(middle_center);
  canvas.setTextSize(2);
  const char* title = "CARDUCTIVE";
  int len = strlen(title);
  for (int i = 1; i <= len; i++) {
    canvas.fillScreen(COL_BG);
    char part[32];
    strncpy(part, title, i);
    part[i] = '\0';

    canvas.setTextColor(COL_ACCENT);
    canvas.drawString(part, 120, 60);
    if (i < len) {
      int w = canvas.textWidth(part);
      canvas.fillRect(120 + (w / 2) + 2, 50, 2, 20, COL_TEXT_NORM);
    }

    canvas.pushSprite(0, 0);
    delay(50);
  }

  delay(600);
  canvas.setTextDatum(top_left);
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  setCpuFrequencyMhz(80);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(BRIGHT_HIGH);

  if (canvas.createSprite(240, 135) == nullptr) {
      M5Cardputer.Display.println("CANVAS ALLOC FAILED!");
      while(1) delay(100);
  }

  playIntro();

  SPI.begin(40, 39, 14, 12);
  bool sdWorks = SD.begin(12, SPI, 25000000);

  if (!sdWorks) {
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_RED);
    canvas.setTextSize(1.5);
    canvas.drawString("NO SD CARD DETECTED", 120, 95);
    canvas.pushSprite(0, 0);
    delay(2000);
  } else {
    if (!SD.exists(DATA_PATH)) {
      SD.mkdir(DATA_PATH);
    }
    loadGlobalDate(); 
  }

  canvas.setTextDatum(top_left);
  
  todoApp.init();
  pomodoroApp.init(); 
  habitApp.init();
  dayTrackApp.init();
  
  lastActivityTime = millis();
  needsRedraw = true;
}

void loop() {
  M5Cardputer.update();
  bool isAction = M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed();
  bool isPomodoro = (currentMode == APP_POMODORO);

if (isAction) {
    lastActivityTime = millis();
    if (isScreenDimmed) {
      setCpuFrequencyMhz(240); 
      
      
      M5Cardputer.Display.setBrightness(BRIGHT_HIGH);
      isScreenDimmed = false;
      needsRedraw = true;
    }
  } else {
    if (!isScreenDimmed && millis() - lastActivityTime > DIM_DELAY_MS) {
      neopixelWrite(21, 150, 0, 0); 
      
      M5Cardputer.Display.setBrightness(BRIGHT_DIM);
      isScreenDimmed = true;
    }
  }

  if (isAction) {
    bool blocked = false;
    if (currentMode == APP_TODO && todoApp.isTypingMode()) blocked = true;
    if (currentMode == APP_HABIT && habitApp.isTypingMode()) blocked = true;
    if (isPomodoro && pomodoroApp.isActive()) blocked = true;

    bool appSwitched = false;
    if (!blocked) {
      if (M5Cardputer.Keyboard.isKeyPressed(',')) {
        if (!(currentMode == APP_HABIT && habitApp.moveColumnLeft())) {
          currentMode = (AppMode)((currentMode - 1 + APP_COUNT) % APP_COUNT);
          appSwitched = true;
          if (currentMode == APP_HABIT) habitApp.setColumn(CAT_EVENING); 
        }
        needsRedraw = true;
      } 
      else if (M5Cardputer.Keyboard.isKeyPressed('/')) {
        if (!(currentMode == APP_HABIT && habitApp.moveColumnRight())) {
          currentMode = (AppMode)((currentMode + 1) % APP_COUNT);
          appSwitched = true;
          if (currentMode == APP_HABIT) habitApp.setColumn(CAT_MORNING); 
        }
        needsRedraw = true;
      }
    }

    if (!appSwitched) {
      handleUpdate();
      needsRedraw = true;
    }
  }

  if (isPomodoro && pomodoroApp.isActive()) {
      handleUpdate();
      needsRedraw = true; 
  }

  if (needsRedraw) {
    canvas.fillScreen(COL_BG);
    handleDraw();

    canvas.fillRect(200, 0, 40, 20, COL_HEADER_BG);
    int bat = M5.Power.getBatteryLevel();
    uint16_t batCol = (bat > 20) ? COL_HIGHLIGHT : COL_P1;

    canvas.drawRect(205, 6, 30, 8, COL_TEXT_NORM);
    canvas.fillRect(207, 8, (bat * 26) / 100, 4, batCol);

    canvas.pushSprite(0, 0);
    needsRedraw = false;
  }

  delay(10); 
}

void wipeData() {
  if (SD.exists(DATA_PATH "/todo.bin")) {
    SD.remove(DATA_PATH "/todo.bin");
    canvas.fillScreen(TFT_RED);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCenterString("SYSTEM RESET!", 120, 60);
    canvas.pushSprite(0, 0);
    delay(1000);
  }
}