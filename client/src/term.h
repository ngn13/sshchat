#pragma once
#include <ncurses.h>
#include <stdbool.h>

typedef struct term {
  WINDOW *win;
  int width, height;
  int x, y;
} term_t;

void term_init();
void term_print(char *, ...);
void term_bold(char *, ...);
void term_boldnl(char *, ...);
void term_printnl(char *, ...);
void term_error(char *, ...);
void term_success(char *, ...);
void term_timenl();
void term_finish();
void term_clear();
