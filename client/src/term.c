#include "term.h"

#include <curses.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/util.h"
#include "input.h"

term_t term;

void term_init() {
  initscr();
  cbreak();
  clear();

  term.win = newwin(LINES - 1, COLS, 0, 0);

  start_color();
  int bg = getbkgd(term.win);
  init_pair(5, COLOR_MAGENTA, bg);
  init_pair(4, COLOR_GREEN, bg);
  init_pair(2, COLOR_BLUE, bg);
  init_pair(3, COLOR_RED, bg);

  keypad(term.win, TRUE);
  wrefresh(term.win);
  input_init();
}

void term_update() {
  getyx(term.win, term.y, term.x);
  getmaxyx(term.win, term.height, term.width);
  wrefresh(input.win);
}

void term_scroll() {
  if (term.y + 1 >= term.height)
    wclear(term.win);
}

void term_print(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  char time[TIME_MAX];
  get_time(time);

  wattron(term.win, COLOR_PAIR(2));
  wprintw(term.win, "%s | ", time);
  wattroff(term.win, COLOR_PAIR(2));

  vw_printw(term.win, text, args);
  wprintw(term.win, "\n");

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_printnl(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  vw_printw(term.win, text, args);

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_bold(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  char time[TIME_MAX];
  get_time(time);

  wattron(term.win, COLOR_PAIR(2));
  wprintw(term.win, "%s | ", time);
  wattroff(term.win, COLOR_PAIR(2));

  wattron(term.win, A_BOLD);
  vw_printw(term.win, text, args);
  wattroff(term.win, A_BOLD);
  wprintw(term.win, "\n");

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_timenl() {
  term_scroll();
  va_list args;
  char time[TIME_MAX];
  get_time(time);

  wattron(term.win, COLOR_PAIR(5));
  wprintw(term.win, "%s | ", time);
  wattroff(term.win, COLOR_PAIR(5));

  wrefresh(term.win);
  term_update();
}

void term_boldnl(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  wattron(term.win, A_BOLD);
  vw_printw(term.win, text, args);
  wattroff(term.win, A_BOLD);

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_error(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  char time[TIME_MAX];
  get_time(time);

  wattron(term.win, COLOR_PAIR(2));
  wprintw(term.win, "%s | ", time);
  wattroff(term.win, COLOR_PAIR(2));

  wattron(term.win, COLOR_PAIR(3));
  vw_printw(term.win, text, args);
  wattroff(term.win, COLOR_PAIR(3));
  wprintw(term.win, "\n");

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_success(char *text, ...) {
  term_scroll();
  va_list args;
  va_start(args, text);

  char time[TIME_MAX];
  get_time(time);

  wattron(term.win, COLOR_PAIR(2));
  wprintw(term.win, "%s | ", time);
  wattroff(term.win, COLOR_PAIR(2));

  wattron(term.win, COLOR_PAIR(4));
  vw_printw(term.win, text, args);
  wattroff(term.win, COLOR_PAIR(4));
  wprintw(term.win, "\n");

  va_end(args);
  wrefresh(term.win);
  term_update();
}

void term_clear() {
  wclear(term.win);
  wrefresh(term.win);
  term_update();
}

void term_refersh() {
  wrefresh(term.win);
}

void term_finish() {
  clear();
  input_free();
  endwin();
}
