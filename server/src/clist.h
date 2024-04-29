#pragma once
#include "client.h"
#include <stdbool.h>
#include <stdlib.h>
#define CLIST_SIZE 16

typedef struct clist {
  client_t *list[CLIST_SIZE];
  size_t len;
} clist_t;

typedef void (*for_func)(client_t *, void *);
void clist_add(client_t *);
bool clist_del(client_t *);
void clist_foreach(for_func, void *);
client_t *clist_get(char *);
bool clist_contains(char *);
size_t clist_len();
void clist_init();
void clist_free();
client_t *clist_get(char *);
void clist_send_all(char *, int);
