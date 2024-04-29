#pragma once
#include <libssh/server.h>
#include <stdbool.h>

#include "clist.h"
#include "msg.h"

typedef void (*cmd_func)(client_t *, msg_t *);
typedef struct cmd {
  cmd_func func;
  char *name;
  bool arg_required;
} cmd_t;

int cmd_data_callback(ssh_session, ssh_channel, void *, uint32_t, int, void *);
