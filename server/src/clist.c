#include "clist.h"
#include "../../lib/util.h"
#include "banlist.h"
#include <stdio.h>
#include <string.h>

clist_t clist;

int clist_hash(char *name) {
  if (NULL == name)
    return -1;

  int indx = 0;
  for (char *cur = name; *cur != '\0'; cur++)
    indx += (int)*cur;
  return indx % CLIST_SIZE;
}

void clist_init() {
  clist.len = 0;
  for (int i = 0; i < CLIST_SIZE; i++)
    clist.list[i] = NULL;
}

void clist_add(client_t *c) {
  int indx = clist_hash(c->user);
  if (indx == -1)
    return;

  clist.len++;

  if (!clist.list[indx]) {
    c->next          = NULL;
    clist.list[indx] = c;
    return;
  }

  client_t *cur = clist.list[indx];
  while (cur->next)
    cur = cur->next;
  cur->next = c;
}

bool clist_del(client_t *c) {
  int indx = clist_hash(c->user);
  if (indx == -1)
    return false;

  if (!clist.list[indx])
    return false;

  client_t *cur = clist.list[indx], *found;
  if (eq(cur->user, c->user)) {
    found            = clist.list[indx];
    clist.list[indx] = cur->next;
    client_free(found);
    clist.len--;
    return true;
  }

  while (cur->next) {
    if (eq(cur->next->user, c->user))
      break;
    cur = cur->next;
  }

  if (!cur->next)
    return false;

  found     = cur->next;
  cur->next = cur->next->next;
  client_free(found);
  clist.len--;
  return true;
}

void clist_foreach(for_func func, void *arg) {
  for (int i = 0; i < CLIST_SIZE; i++) {
    client_t *cur = clist.list[i], *prev;
    while (cur) {
      prev = cur;
      cur  = cur->next;
      func(prev, arg);
    }
  }
}

client_t *clist_get(char *name) {
  int indx = clist_hash(name);
  if (indx == -1)
    return NULL;

  client_t *cur = clist.list[indx];
  while (cur) {
    if (eq(cur->user, name))
      return cur;
    cur = cur->next;
  }

  return NULL;
}

bool clist_contains(char *name) {
  return clist_get(name) != NULL;
}

size_t clist_len() {
  return clist.len;
}

void clist_free_client(client_t *c, void *arg) {
  client_free(c);
}

void clist_free() {
  clist_foreach(clist_free_client, NULL);
}

void clist_send_client(client_t *c, void *arg) {
  struct tmp_arg {
    char *msg;
    int len;
  };
  struct tmp_arg *data = arg;
  client_send(c, data->msg, data->len);
}

void clist_send_all(char *msg, int len) {
  struct tmp_arg {
    char *msg;
    int len;
  };
  struct tmp_arg arg = {.msg = msg, .len = len};
  clist_foreach(clist_send_client, &arg);
}
