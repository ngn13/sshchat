#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/util.h"
#include "banlist.h"

#define MATCH(e) entry_match(e, ipstr, user)
ban_entry_t *first = NULL;

void banlist_add(char *ipstr, char *user) {
  ban_entry_t *en = malloc(sizeof(ban_entry_t));
  en->next        = NULL;

  if (NULL != ipstr)
    en->ipstr = strdup(ipstr);
  else
    en->ipstr = NULL;

  if (NULL != user)
    en->user = strdup(user);
  else
    en->user = NULL;

  if (NULL == first) {
    first = en;
    return;
  }

  ban_entry_t *cur = first;
  while (cur->next)
    cur = cur->next;
  cur->next = en;
}

bool entry_match(ban_entry_t *en, char *ipstr, char *user) {
  if (NULL != ipstr && NULL != en->ipstr)
    if (eq(ipstr, en->ipstr))
      return true;

  if (NULL != user && NULL != en->user)
    if (eq(user, en->user))
      return true;

  return false;
}

void entry_free(ban_entry_t *en) {
  if (NULL == en)
    return;

  free(en->ipstr);
  free(en->user);
  free(en);
}

bool banlist_contains(char *ipstr, char *user) {
  ban_entry_t *cur = first;
  while (cur) {
    if (MATCH(cur))
      return true;
    cur = cur->next;
  }
  return false;
}

bool banlist_del(char *ipstr, char *user) {
  ban_entry_t *cur = first, *prev = first;
  if (NULL == cur)
    return false;

  while (cur) {
    if (MATCH(cur))
      break;
    prev = cur;
    cur  = cur->next;
  }

  prev->next = cur->next;
  entry_free(cur);
  return true;
}

void banlist_free() {
  ban_entry_t *cur = first, *prev = NULL;
  while (cur) {
    prev = cur;
    cur  = cur->next;
    free(prev);
  }
}
