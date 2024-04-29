#include "history.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

history_t history = {
    .last   = NULL,
    .first  = NULL,
    .length = 0,
};

void history_free() {
  history_entry_t *ent = history.first, *prev = NULL;
  while (ent) {
    prev = ent;
    ent  = ent->next;

    free(prev->msg);
    free(prev);
  }
}

void history_pop() {
  if (NULL == history.first)
    return;

  history_entry_t *ent = history.first;
  free(ent->msg);
  history.first = ent->next;
  free(ent);
}

void history_add(char *msg, int len) {
  int histsize = config_get_int("chat/history");
  if (histsize == 0)
    return;

  history_entry_t *entry = malloc(sizeof(history_entry_t));
  entry->msg             = strdup(msg);
  entry->next            = NULL;
  entry->len             = len;

  if (NULL == history.first || NULL == history.last) {
    history.first = entry;
    history.last  = entry;
    return;
  }

  history.last->next = entry;
  history.last       = entry;
  history.length++;

  if (history.length > histsize)
    history_pop();
}

history_entry_t *history_first() {
  return history.first;
}
