#include "client.h"

#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "term.h"

client_t client;

bool client_init(char *arg) {
  client.channel = NULL;
  client.session = NULL;
  client.event   = NULL;
  client.user    = NULL;
  client.ip      = NULL;
  client.quit    = false;
  client.port    = 2222;

  pthread_mutex_init(&client.lock, NULL);

  char *save, *host = NULL, *sport = NULL;
  int port = 2222;

  client.user = strtok_r(arg, "@", &save);
  if (NULL == client.user) {
    term_error("Failed to parse the username");
    return false;
  }

  host = strtok_r(NULL, "@", &save);
  if (NULL == host) {
    host = client.user;

    struct passwd *pw;
    uid_t uid;
    pw          = getpwuid(geteuid());
    client.user = pw->pw_name;
  }

  client.ip = strtok_r(host, ":", &save);
  if (NULL == client.ip) {
    client.ip = host;
    return true;
  }

  sport = strtok_r(NULL, ":", &save);
  if (NULL == sport)
    return true;

  client.port = atoi(sport);
  if (client.port <= 0) {
    term_error("Invalid port number: %d", client.port);
    return false;
  }

  return true;
}

bool client_is_alive() {
  return ssh_is_connected(client.session) && ssh_channel_is_open(client.channel);
}

void client_free() {
  if (NULL != client.channel) {
    ssh_channel_close(client.channel);
    ssh_channel_free(client.channel);
  }

  if (NULL != client.event) {
    ssh_event_remove_session(client.event, client.session);
    ssh_event_free(client.event);
  }

  if (NULL != client.session) {
    ssh_disconnect(client.session);
    ssh_free(client.session);
  }

  pthread_mutex_unlock(&client.lock);
  pthread_mutex_destroy(&client.lock);
}
