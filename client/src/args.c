#include "args.h"

#include <stdlib.h>
#include <string.h>

void args_parse(args_t *args, char *cmd) {
  int cmdlen     = strlen(cmd) + 1;
  args->has_data = false;
  args->base     = malloc(cmdlen);
  args->data     = malloc(cmdlen);

  args->base_len = 0;
  args->data_len = 0;

  char *c  = cmd;
  int indx = 0;

  for (; *c != ' ' && *c != '\0'; c++) {
    args->base[indx] = *c;
    args->base_len++;
    indx++;
  }

  args->base[indx] = '\0';
  indx             = 0;
  c++;

  for (; *c != '\0'; c++) {
    args->data[indx] = *c;
    args->data_len++;
    indx++;
  }

  if (0 != indx)
    args->has_data = true;
  args->data[indx] = '\0';
}

void args_free(args_t *args) {
  free(args->base);
  free(args->data);
}
