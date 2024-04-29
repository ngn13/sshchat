#include "msg.h"
#include <stdlib.h>
#include <string.h>

char *msg_get_arg(char *cmd, int len) {
  char *arg = malloc(len);
  bzero(arg, len);
  int indx = 0;

  for (int i = 0; i < len; i++) {
    if (i <= 3)
      continue;
    arg[indx] = cmd[i];
    indx++;
  }

  return arg;
}

void msg_load(msg_t *m, char *text, int len) {
  m->cmd     = text;
  m->cmd_len = len;

  m->arg     = msg_get_arg(text, len);
  m->arg_len = strlen(m->arg);
}

void msg_free(msg_t *m) {
  free(m->arg);
}
