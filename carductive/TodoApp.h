#ifndef TODOAPP_H
#define TODOAPP_H

#include <Arduino.h>
#include <vector>

struct TodoItem {
  char text[20];
  int priority;
  int urgency;
  bool done;
};

class TodoApp {
 public:
  void init();
  void update();
  void draw();

  bool isTypingMode();

 private:
  std::vector<TodoItem> todoList;
  int selectedIndex = 0;
  bool isTyping = false;
  bool isReordering = false;

  char inputBuffer[64] = {0};
  const char* SEP_TAG = "---";

  void loadFromSD();
  void saveToSD();
  void addTask(const char* text);
  void moveTask(int direction);
  void deleteTask();
  void toggleDone();

  void adjustVal(int& field, int delta, int minV, int maxV);
};

#endif