#include <arpa/inet.h>
#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/log.h"
#include "../../lib/util.h"
#include "banlist.h"
#include "cmd.h"
#include "config.h"
#include "session.h"

bool session_is_invalid_username(const char *user, int len) {
  if (len > config_get_int("chat/usermax") || len < config_get_int("chat/usermin"))
    return true;

  for (char *c = (char *)user; *c != '\0'; c++) {
    int v = (int)*c;
    if (v < 48 || (v > 57 && v < 65) || (v > 90 && v < 97) || v > 122)
      return true;
  }

  return clist_contains((char *)user);
}

int session_auth_callback(ssh_session session, const char *user, const char *pass, void *data) {
  client_t *c = (client_t *)data;

  c->user_len = strlen(user);
  if (session_is_invalid_username(user, c->user_len)) {
    c->auth_tries++;
    debug("(%s) Auth failed because the username is invalid or it's taken", c->ipstr);
    return SSH_AUTH_PARTIAL;
  }

  if (config_get_str("security/password") != NULL && !eq(config_get_str("security/password"), (char *)pass)) {
    c->auth_tries++;
    return SSH_AUTH_ERROR;
  }

  c->user     = strdup(user);
  c->password = strdup(pass);

  debug("(%s) Auth request with the credentials %s:%s", c->ipstr, user, pass);
  return SSH_AUTH_SUCCESS;
}

ssh_channel session_channel_callback(ssh_session session, void *data) {
  client_t *c = (client_t *)data;
  c->channel  = ssh_channel_new(c->session);
  return c->channel;
}

void *session_handle(void *arg) {
  client_t *c = (client_t *)arg;

  struct ssh_server_callbacks_struct server_callbacks = {
      .userdata                              = c,
      .auth_password_function                = session_auth_callback,
      .channel_open_request_session_function = session_channel_callback,
  };
  ssh_callbacks_init(&server_callbacks);
  ssh_set_server_callbacks(c->session, &server_callbacks);

  if (ssh_handle_key_exchange(c->session) != 0) {
    debug("(%s) Failed to exchange keys: %s", c->ipstr, ssh_get_error(c->server));
    goto END;
  }

  c->event = ssh_event_new();
  ssh_event_add_session(c->event, c->session);

  int max_retries = config_get_int("security/retries") + 1;
  if (max_retries <= 0)
    max_retries = 4;

  while (NULL == c->channel) {
    if (ssh_event_dopoll(c->event, 100) == SSH_ERROR) {
      debug("(%s) Event poll failed: %s", c->ipstr, ssh_get_error(c->session));
      goto END;
    }

    if (c->auth_tries > config_get_int("security/retries")) {
      debug("(%s) Client reached max password retry limit, disconnecting", c->ipstr);
      goto END;
    }
  }

  if (banlist_contains(c->ipstr, c->user)) {
    char *msg = "You have been banned from this server";
    client_send(c, msg, strlen(msg) + 1);
    debug("(%s@%s) Ending connection because the user is banned", c->user, c->ipstr);
    goto END;
  }

  while (clist_len() >= config_get_int("security/maxcon")) {
    if (!ssh_channel_is_open(c->channel))
      goto END;

    char *msg = "Server is full, waiting for someone to leave";
    client_send(c, msg, strlen(msg) + 1);
    sleep(10);
  }

  struct ssh_channel_callbacks_struct channel_callbacks = {
      .userdata              = c,
      .channel_data_function = cmd_data_callback,
  };
  ssh_callbacks_init(&channel_callbacks);
  ssh_set_channel_callbacks(c->channel, &channel_callbacks);
  clist_add(c);

  char *joined_fmt = config_get_str("chat/joinmsg");
  char *joined_msg = malloc(c->user_len + strlen(joined_fmt) + 1);
  sprintf(joined_msg, joined_fmt, c->user);
  clist_send_all(joined_msg, strlen(joined_msg) + 1);
  free(joined_msg);

  char *motd = config_get_str("chat/motd");
  if (NULL != motd)
    client_send(c, motd, strlen(motd));

  while (ssh_channel_is_open(c->channel)) {
    if (ssh_event_dopoll(c->event, -1) == SSH_ERROR) {
      debug("(%s@%s) Closing channel: %s", c->user, c->ipstr, ssh_get_error(c->session));
      break;
    }
  }

  char *left_fmt = config_get_str("chat/leftmsg");
  char *left_msg = malloc(c->user_len + strlen(left_fmt) + 1);
  sprintf(left_msg, left_fmt, c->user);
  clist_send_all(left_msg, strlen(left_msg) + 1);
  free(left_msg);

END:
  if (c->user != NULL)
    debug("(%s@%s) Closing session", c->user, c->ipstr);
  else
    debug("(%s) Closing session", c->ipstr);

  client_clean(c);
  clist_del(c);
  return NULL;
}

void session_getip(ssh_session session, char *ip) {
  struct sockaddr addr;
  socklen_t address_len;
  getpeername(ssh_get_fd(session), &addr, &address_len);

  if (addr.sa_family == AF_INET) {
    struct in_addr *ipv4 = &((struct sockaddr_in *)&addr)->sin_addr;
    inet_ntop(addr.sa_family, ipv4, ip, INET6_ADDRSTRLEN);
    return;
  }

  struct in6_addr *ipv6 = &((struct sockaddr_in6 *)&addr)->sin6_addr;
  inet_ntop(addr.sa_family, ipv6, ip, INET6_ADDRSTRLEN);
}
