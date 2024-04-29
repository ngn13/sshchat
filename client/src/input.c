#include "input.h"

#include <ncurses.h>
#include <string.h>

#include "../../lib/util.h"
#include "term.h"

input_t input;

void input_init() {
  input.win    = newwin(1, COLS, LINES - 1, 0);
  input.prompt = NULL;
  input.active = NULL;

  keypad(input.win, TRUE);
  wrefresh(input.win);
}

void input_prompt_display() {
  if (NULL == input.prompt)
    return;

  wclear(input.win);

  wattron(input.win, A_BOLD);
  wprintw(input.win, "[%s]# ", input.prompt);
  wattroff(input.win, A_BOLD);

  wrefresh(input.win);
}

void input_prompt(char *prompt, ...) {
  va_list args, argscp;
  va_start(args, prompt);
  va_copy(argscp, args);

  int sz    = vsnprintf(NULL, 0, prompt, args);
  char *msg = malloc(sz);
  vsprintf(msg, prompt, argscp);

  input.prompt = msg;
  input_prompt_display();
  va_end(args);
}

void input_update() {
  getyx(input.win, input.y, input.x);
  getmaxyx(input.win, input.height, input.width);
}

void input_prompt_free() {
  free(input.prompt);
  input.prompt = NULL;
}

void input_buffer_display() {
  if (input.size == 0 || NULL == input.buffer) {
    wprintw(input.win, "");
    wrefresh(input.win);
    return;
  }

  for (int i = 0; i < input.size; i++) {
    if (input.hidden)
      wprintw(input.win, "*");
    else
      wprintw(input.win, "%c", input.buffer[i]);
  }
}

void input_handle_del() {
  if (input.size <= 0 || NULL == input.buffer || input.cursor == input.size)
    return;

  input.size--;
  int oldx = input.x;
  int indx = input.cursor;

  if (indx < 0)
    return;

  if (input.size <= 0) {
    free(input.buffer);
    input.buffer = NULL;
    input_prompt_display();
    input_buffer_display();
    return;
  }

  char *new    = malloc(input.size);
  int new_indx = 0;

  for (int i = 0; i < input.size + 1; i++) {
    if (indx == i)
      continue;

    new[new_indx] = input.buffer[i];
    new_indx++;
  }

  free(input.buffer);
  input.buffer = new;

  input_prompt_display();
  input_buffer_display();

  input.x = oldx;
}

void input_handle_backspace() {
  if (input.size <= 0 || NULL == input.buffer || input.cursor == 0)
    return;

  input.size--;
  int oldx = --input.x;
  int indx = --input.cursor;

  if (indx < 0)
    return;

  if (input.size <= 0) {
    free(input.buffer);
    input.buffer = NULL;
    input_prompt_display();
    input_buffer_display();
    return;
  }

  char *new    = malloc(input.size);
  int new_indx = 0;

  for (int i = 0; i < input.size + 1; i++) {
    if (indx == i)
      continue;

    new[new_indx] = input.buffer[i];
    new_indx++;
  }

  free(input.buffer);
  input.buffer = new;

  input_prompt_display();
  input_buffer_display();

  input.x = oldx;
}

bool input_handle_custom(int c) {
  switch (c) {
  case KEY_BACKSPACE:
    input_handle_backspace();
    break;
  case KEY_DC:
    input_handle_del();
    break;
  case 127:
    input_handle_backspace();
    break;

  case KEY_UP:
    break;
  case KEY_DOWN:
    break;

  case KEY_RIGHT:
    if (input.cursor >= input.size) {
      input.cursor = input.size;
      break;
    }

    input.cursor++;
    input.x++;
    break;

  case KEY_LEFT:
    if (input.cursor <= 0) {
      input.cursor = 0;
      break;
    }

    input.cursor--;
    input.x--;
    break;

  case 1:
    input.x -= input.cursor;
    input.cursor = 0;
    break;

  case 5:
    input.x += input.size - input.cursor;
    input.cursor = input.size;
    break;

  default:
    return false;
    break;
  }

  wmove(input.win, input.y, input.x);
  wrefresh(input.win);
  return true;
}

void input_buffer_add(int c) {
  input.cursor++;
  input.size++;

  if (input.cursor == input.size) {
    if (NULL == input.buffer)
      input.buffer = malloc(input.size);
    else
      input.buffer = realloc(input.buffer, input.size);

    input.buffer[input.size - 1] = c;

    if (input.hidden)
      wprintw(input.win, "*");
    else
      wprintw(input.win, "%c", c);

    return;
  }

  char *new = malloc(input.size);
  int i1 = 0, i2 = 0;

  while (true) {
    if (i1 >= input.size)
      break;

    if (i1 == input.cursor - 1) {
      new[i1] = c;
      i1++;
      continue;
    }

    if (NULL == input.buffer)
      continue;

    new[i1] = input.buffer[i2];
    i2++;
    i1++;
  }

  int oldx = ++input.x;

  free(input.buffer);
  input.buffer = new;

  input_prompt_display();
  input_buffer_display();

  input.x = oldx;
  wmove(input.win, input.y, input.x);
  wrefresh(input.win);
}

char *input_get(bool hidden) {
  if (input.active)
    return NULL;

  input.hidden = hidden;
  input.active = true;
  input.buffer = NULL;
  input.cursor = 0;
  input.size   = 0;
  int c        = 0;

  noecho();

  while ((c = wgetch(input.win)) != '\n') {
    input_update();

    if (!input.active) {
      free(input.buffer);
      input.buffer = NULL;
      goto END;
    }

    if (input_handle_custom(c))
      continue;
    input_buffer_add(c);

    if ((input.x - input.cursor) + input.size >= input.width - 1) {
      // TODO: handle the input when the window is full
    }
  }

  input.buffer                 = realloc(input.buffer, ++input.size);
  input.buffer[input.size - 1] = '\0';

END:
  echo();

  input_prompt_display();
  input.active = false;
  return input.buffer;
}

bool input_yn() {
  char *prompt = input.prompt;
  input_prompt("yes/no");
  bool ret;

RETRY:
  ret       = false;
  char *res = input_get(false);

  if (eq(res, "yes") || eq(res, "YES"))
    ret = true;

  else if (eq(res, "no") || eq(res, "NO"))
    ret = false;

  else {
    term_error("Please answer with \"yes\" or \"no\"");
    free(res);
    goto RETRY;
  }

  free(res);
  input_prompt_free();

  input.prompt = prompt;
  input_prompt_display();

  return ret;
}

void input_pause() {
  term_bold("Hit any key to exit");
  while (wgetch(input.win) != '\n')
    continue;
}

void input_free() {
  input.active = false;
  free(input.prompt);
}
