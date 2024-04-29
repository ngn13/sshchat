#pragma once
#include <stdlib.h>

typedef struct history_entry {
  struct history_entry *next;
  char *msg;
  int len;
} history_entry_t;

typedef struct history {
  history_entry_t *first;
  history_entry_t *last;
  size_t length;
} history_t;

void history_add(char *, int);
history_entry_t *history_first();
void history_free();
