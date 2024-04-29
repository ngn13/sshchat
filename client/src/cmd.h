#pragma once
#include <stdlib.h>

#include "args.h"

enum cmd_ret {
  RET_BREAK    = -1,
  RET_INVALID  = -2,
  RET_CONTINUE = -3,
};

typedef int (*func)(args_t *);
typedef struct cmd {
  bool arg_required;
  char *name;
  func func;
} cmd_t;

int cmd_run(char *);
int cmd_help(args_t *);
