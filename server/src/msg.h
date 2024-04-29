#pragma once

typedef struct msg {
  char *cmd;
  char *arg;

  int cmd_len;
  int arg_len;
} msg_t;

void msg_load(msg_t *, char *, int);
void msg_free(msg_t *);
