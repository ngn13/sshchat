#pragma once
#include <stdbool.h>

enum option_type {
  TYPE_STR  = 1,
  TYPE_INT  = 2,
  TYPE_BOOL = 3,
};

typedef struct option {
  char *key;
  int type;
  char *min;
  char *max;
  char *value;

  bool updated;
  bool verified;

  char *value_char;
  bool value_bool;
  int value_int;
} option_t;

bool config_load(char *);
int config_get_int(char *);
bool config_get_bool(char *);
char *config_get_str(char *);
void config_free();
