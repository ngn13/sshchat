#pragma once
#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct input {
  WINDOW *win;

  char *prompt;
  char *buffer;

  size_t cursor;
  size_t size;
  bool active;

  int width, height;
  bool hidden;
  int x, y;
} input_t;

extern input_t input;

void input_init();
char *input_get(bool);
void input_prompt(char *, ...);
void input_prompt_free();
void input_update();
void input_pause();
void input_free();
bool input_yn();
