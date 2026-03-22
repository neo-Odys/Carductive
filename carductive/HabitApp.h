#ifndef HABITAPP_H
#define HABITAPP_H

#include <Arduino.h>

#include <vector>

enum HabitCat { CAT_MORNING, CAT_AFTERNOON, CAT_EVENING, CAT_COUNT };

struct HabitItem {
  char text[32];
  bool done;
};

class HabitApp {
 public:
  void init();
  void update();
  void draw();
  bool isTypingMode();

  bool moveColumnLeft();
  bool moveColumnRight();
  void setColumn(int col);

 private:
  std::vector<HabitItem> columns[CAT_COUNT];

  int currentColumn = 0;
  int selectedIndex = 0;

  bool isTyping = false;
  bool isReordering = false;
  char inputBuffer[32] = {0};

  const char* catNames[CAT_COUNT] = {"MORNING", "AFTERNOON", "EVENING"};

  void toggleHabit();
  void addHabit(const char* text);
  void deleteHabit();
  void moveHabit(int direction);
  void changeDate(int delta);

  void saveMaster();
  void loadMaster();

  void loadDay();
  void appendHabitState(int cat, const HabitItem& item);
};

#endif