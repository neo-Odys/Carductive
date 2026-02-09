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
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(BRIGHT_HIGH);

  canvas.createSprite(240, 135);

  playIntro();
  bool sdWorks = SD.begin(GPIO_NUM_12, SPI, 25000000);

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
  }

  canvas.setTextDatum(top_left);
  todoApp.init();

  lastActivityTime = millis();
}

void loop() {
  M5Cardputer.update();

  bool isAction =
      M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed();

  if (isAction) {
    lastActivityTime = millis();
    if (isScreenDimmed) {
      M5Cardputer.Display.setBrightness(BRIGHT_HIGH);
      isScreenDimmed = false;
    }
  } else {
    if (!isScreenDimmed && (millis() - lastActivityTime > DIM_DELAY_MS)) {
      M5Cardputer.Display.setBrightness(BRIGHT_DIM);
      isScreenDimmed = true;
    }
  }

  bool isPomodoro = (currentMode == APP_POMODORO);
  bool blocked = false;
  if (currentMode == APP_TODO && todoApp.isTypingMode()) blocked = true;
  if (isPomodoro && pomodoroApp.isActive()) blocked = true;

  bool appSwitched = false;

  if (isAction && !blocked) {
    if (M5Cardputer.Keyboard.isKeyPressed(',')) {
      currentMode = (AppMode)((currentMode - 1 + APP_COUNT) % APP_COUNT);
      appSwitched = true;
    } else if (M5Cardputer.Keyboard.isKeyPressed('/')) {
      currentMode = (AppMode)((currentMode + 1) % APP_COUNT);
      appSwitched = true;
    }
  }

  if (!appSwitched) {
    if (isPomodoro) {
      handleUpdate();
    } else {
      if (isAction) {
        handleUpdate();
      }
    }
  }

  canvas.fillScreen(COL_BG);
  handleDraw();

  // Battery
  canvas.fillRect(200, 0, 40, 20, COL_HEADER_BG);
  int bat = M5.Power.getBatteryLevel();
  uint16_t batCol = (bat > 20) ? COL_HIGHLIGHT : COL_P1;

  canvas.drawRect(205, 6, 30, 8, COL_TEXT_NORM);
  canvas.fillRect(207, 8, (bat * 26) / 100, 4, batCol);

  canvas.pushSprite(0, 0);
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