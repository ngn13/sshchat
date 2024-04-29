#pragma once
#include <libssh/server.h>
#include <stdbool.h>

typedef struct client {
  ssh_bind server;
  ssh_session session;
  ssh_channel channel;
  ssh_event event;
  struct client *next;
  size_t user_len;
  int auth_tries;
  char *ipstr;
  char *user;
  char *password;
  bool kicked;
  bool is_admin;
} client_t;

client_t *client_new(ssh_bind, ssh_session);
void client_send(client_t *, char *, int);
void client_clean(client_t *);
void client_free(client_t *);
void client_kill(client_t *);
