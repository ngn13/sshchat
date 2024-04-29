#pragma once

typedef struct ban_entry {
  struct ban_entry *next;
  char *ipstr;
  char *user;
} ban_entry_t;

void banlist_add(char *, char *);
bool banlist_contains(char *, char *);
bool banlist_del(char *, char *);
void banlist_free();
