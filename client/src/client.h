#pragma once
#include <libssh/libssh.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct Client {
  pthread_mutex_t lock;
  ssh_session session;
  ssh_channel channel;
  ssh_event event;
  bool quit;
  char *user;
  char *ip;
  int port;
} client_t;

extern client_t client;
bool client_init(char *);
bool client_is_alive();
void client_free();
