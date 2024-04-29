#include "client.h"
#include <libssh/libssh.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

client_t *client_new(ssh_bind serv, ssh_session sess) {
  client_t *c = malloc(sizeof(client_t));

  c->session = sess;
  c->server  = serv;

  c->channel  = NULL;
  c->ipstr    = NULL;
  c->event    = NULL;
  c->user     = NULL;
  c->password = NULL;
  c->next     = NULL;

  c->user_len   = 0;
  c->auth_tries = 0;

  c->is_admin = false;
  c->kicked   = false;

  return c;
}

void client_send(client_t *c, char *msg, int len) {
  if (NULL == c->channel)
    return;

  if (!ssh_is_connected(c->session))
    return;

  ssh_channel_write(c->channel, msg, len);
}

void client_kill(client_t *c) {
  if (NULL != c->channel)
    ssh_channel_close(c->channel);
}

void client_clean(client_t *c) {
  if (NULL != c->channel) {
    ssh_channel_send_eof(c->channel);
    ssh_channel_free(c->channel);
  }

  if (NULL != c->event) {
    ssh_event_remove_session(c->event, c->session);
    ssh_event_free(c->event);
  }

  if (NULL != c->session)
    ssh_disconnect(c->session);
}

void client_free(client_t *c) {
  ssh_free(c->session);
  free(c->ipstr);
  free(c->user);
  free(c->password);
  free(c);
}
