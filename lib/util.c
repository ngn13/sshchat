#include "util.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool eq(char *s1, char *s2) {
  if (strlen(s1) != strlen(s2))
    return false;
  return strcmp(s1, s2) == 0;
}

bool startswith(char *str, char *sub) {
  if (strlen(sub) > strlen(str))
    return false;
  return strncmp(str, sub, strlen(sub)) == 0;
}

bool endswith(char *str, char *sub) {
  int strl = strlen(str);
  int subl = strlen(sub);

  if (subl > strl)
    return false;
  return strncmp(str + strl - subl, sub, subl) == 0;
}

int digits(int n) {
  if (n < 0)
    return digits((n == INT_MIN) ? INT_MAX : -n);
  if (n < 10)
    return 1;
  return 1 + digits(n / 10);
}

void get_time(char *res) {
  time_t now   = time(NULL);
  struct tm tm = *localtime(&now);
  sprintf(res, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}
