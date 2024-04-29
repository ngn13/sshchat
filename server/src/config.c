#include <ini.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/log.h"
#include "../../lib/util.h"
#include "config.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
option_t config[] = {
    {.key = "general/debug", .value = "false", .type = TYPE_BOOL},
    {.key = "general/host", .value = "0.0.0.0", .type = TYPE_STR},
    {.key = "general/port", .value = "2222", .type = TYPE_INT, .min = "1", .max = "65535"},
    {.key = "general/key", .value = "/etc/sshchat/key.priv", .type = TYPE_STR},

    {.key = "security/password", .value = NULL, .type = TYPE_STR},
    {.key = "security/retries", .value = "3", .type = TYPE_INT, .min = "1", .max = "-1"},
    {.key = "security/maxcon", .value = "99", .type = TYPE_INT, .min = "1", .max = "-1"},

    {.key = "admin/password", .value = NULL, .type = TYPE_STR},
    {.key = "admin/hide", .value = "false", .type = TYPE_BOOL},
    {.key = "admin/enable_ban", .value = "true", .type = TYPE_BOOL},
    {.key = "admin/enable_kick", .value = "true", .type = TYPE_BOOL},
    {.key = "admin/enable_banip", .value = "false", .type = TYPE_BOOL},

    {.key = "chat/motd", .value = NULL, .type = TYPE_STR},
    {.key = "chat/joinmsg", .value = "-> %s joined the server", .type = TYPE_STR},
    {.key = "chat/leftmsg", .value = "<- %s left the server", .type = TYPE_STR},
    {.key = "chat/usermax", .value = "20", .type = TYPE_INT, .min = "1", .max = "-1"},
    {.key = "chat/usermin", .value = "3", .type = TYPE_INT, .min = "1", .max = "-1"},
    {.key = "chat/history", .value = "20", .type = TYPE_INT, .min = "0", .max = "-1"},
};

void option_free(option_t *op) {
  if (op->updated)
    free(op->value);
}

bool option_verify(option_t *op) {
  if (op->verified)
    return true;
  op->verified = true;

  if (op->type == TYPE_STR) {
    op->value_char = op->value;
    return true;
  }

  else if (op->type == TYPE_INT) {
    int val = atoi(op->value);
    int max = atoi(op->max);
    int min = atoi(op->min);

    if (min >= 0 && min > val) {
      error("Bad value for the \"%s\" option, must be larger than %d", op->key, min);
      return false;
    }

    if (max >= 0 && max < val) {
      error("Bad value for the \"%s\" option, must be smaller than %d", op->key, min);
      return false;
    }

    op->value_int = val;
    return true;
  }

  else if (op->type == TYPE_BOOL) {
    if (eq(op->value, "true")) {
      op->value_bool = true;
      return true;
    }

    else if (eq(op->value, "false")) {
      op->value_bool = false;
      return true;
    }

    error("Bad value for the \"%s\" option, must be set to \"true\" or \"false\"", op->key);
    return false;
  }

  error("Invalid type for the \"%s\" option", op->key);
  return false;
}

option_t *config_find(char *key) {
  option_t *op = NULL;
  for (int i = 0; i < sizeof(config) / sizeof(option_t); i++) {
    op = &config[i];
    if (eq(key, op->key))
      return op;
  }
  return NULL;
}

int config_get_int(char *key) {
  option_t *opt = config_find(key);
  if (NULL != opt)
    return opt->value_int;
  return -1;
}

bool config_get_bool(char *key) {
  option_t *opt = config_find(key);
  if (NULL != opt)
    return opt->value_bool;
  return false;
}

char *config_get_str(char *key) {
  option_t *opt = config_find(key);
  if (NULL != opt)
    return opt->value_char;
  return NULL;
}

int config_handle(void *data, const char *section, const char *key, const char *value) {
  char fk[strlen(key) + strlen(section) + 2];
  sprintf(fk, "%s/%s", section, key);

  option_t *op = NULL;
  for (int i = 0; i < sizeof(config) / sizeof(option_t); i++) {
    op = &config[i];
    if (eq(fk, op->key))
      break;
    op = NULL;
  }

  if (NULL == op) {
    error("Unknown option: \"%s\"", fk);
    return false;
  }

  op->value   = strdup(value);
  op->updated = true;
  return !option_verify(op);
}

void config_free() {
  for (int i = 0; i < sizeof(config) / sizeof(option_t); i++)
    option_free(&config[i]);
}

bool config_load(char *path) {
  for (int i = 0; i < sizeof(config) / sizeof(option_t); i++) {
    config[i].verified = false;
    config[i].updated  = false;
  }

  if (ini_parse(path, config_handle, NULL) < 0) {
    error("Failed to load the config: %s", path);
    return false;
  }

  for (int i = 0; i < sizeof(config) / sizeof(option_t); i++)
    option_verify(&config[i]);

  DEBUG = config_get_bool("general/debug");
  return true;
}
