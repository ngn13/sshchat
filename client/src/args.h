#pragma once
#include <stdbool.h>
#include <stdlib.h>

typedef struct args {
  char *base;
  char *data;

  bool has_data;
  size_t base_len;
  size_t data_len;
} args_t;

void args_parse(args_t *, char *);
void args_free(args_t *);
