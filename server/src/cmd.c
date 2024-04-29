#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/log.h"
#include "../../lib/util.h"
#include "banlist.h"
#include "clist.h"
#include "cmd.h"
#include "config.h"
#include "history.h"

void cmd_send(client_t *c, msg_t *m) {
  int msglen = m->cmd_len + strlen(c->user) + 10;
  char usermsg[msglen];
  bzero(usermsg, msglen);

  if (c->is_admin && !config_get_bool("admin/hide"))
    sprintf(usermsg, "<[admin]%s> %s", c->user, m->arg);
  else
    sprintf(usermsg, "<%s> %s", c->user, m->arg);

  int len = strlen(usermsg);
  clist_send_all(usermsg, len + 1);

  history_add(usermsg, len);
}

void cmd_list_client(client_t *c, void *arg) {
  char *list_msg = arg;
  if (c->is_admin && !config_get_bool("admin/hide"))
    sprintf(list_msg, "%s[admin]%s ", list_msg, c->user);
  else
    sprintf(list_msg, "%s%s ", list_msg, c->user);
}

void cmd_list(client_t *c, msg_t *m) {
  char connected_msg[70 + digits(clist_len())];
  if (clist_len() == 1) {
    sprintf(connected_msg, "Currently there is only 1 user connected (and it's you)");
    client_send(c, connected_msg, strlen(connected_msg) + 1);
    return;
  }

  sprintf(connected_msg, "Currently there are %lu users connected", clist_len());
  char *list_msg = malloc(clist_len() * (config_get_int("chat/usermax") + 12));
  sprintf(list_msg, "Here is a full list: ");
  clist_foreach(cmd_list_client, list_msg);

  client_send(c, connected_msg, strlen(connected_msg) + 1);
  client_send(c, list_msg, strlen(list_msg) + 1);
  free(list_msg);
}

void cmd_history(client_t *c, msg_t *m) {
  history_entry_t *ent = history_first();
  while (ent) {
    client_send(c, ent->msg, ent->len + 1);
    ent = ent->next;
  }
}

void cmd_admin_log(client_t *c, msg_t *m) {
  if (c->is_admin) {
    char *already_msg = "You are already an admin";
    client_send(c, already_msg, strlen(already_msg) + 1);
    return;
  }

  char *password = config_get_str("admin/password");
  if (NULL == password) {
    char *disabled_msg = "Failed, admin login is disabled";
    client_send(c, disabled_msg, strlen(disabled_msg) + 1);
    return;
  }

  if (!eq(m->arg, password)) {
    char *fail_msg = "Password is incorrect, login failed";
    client_send(c, fail_msg, strlen(fail_msg) + 1);
    return;
  }

  c->is_admin       = true;
  char *success_msg = "Password is corrrect, you are now and admin";
  client_send(c, success_msg, strlen(success_msg) + 1);
  info("%s is authenticated as admin", c->user);
}

void cmd_ban(client_t *c, msg_t *m) {
  if (!c->is_admin || !config_get_bool("admin/enable_ban")) {
    char *denied_msg = "You don't have access to this command";
    client_send(c, denied_msg, strlen(denied_msg) + 1);
    return;
  }

  client_t *target = clist_get(m->arg);
  if (NULL == target) {
    char *notfound_msg = "User not found";
    client_send(c, notfound_msg, strlen(notfound_msg) + 1);
    return;
  }

  if (target->is_admin) {
    char *perm_msg = "You cannot ban another admin";
    client_send(c, perm_msg, strlen(perm_msg) + 1);
    info("%s tried to ban another admin (%s)", c->user, target->user);
    return;
  }

  target->kicked = true;

  char *banned_msg  = "You have been banned!";
  char *success_msg = "User has been banned";

  info("%s was banned by %s", target->user, c->user);
  client_send(target, banned_msg, strlen(banned_msg) + 1);
  client_send(c, success_msg, strlen(success_msg) + 1);

  banlist_add(NULL, target->user);
  client_kill(target);
}

void cmd_banip_client(client_t *target, void *arg) {
  client_t *c = arg;
  if (!banlist_contains(target->ipstr, NULL))
    return;

  if (target->is_admin)
    return;

  char *banned_msg = "You have been banned!";
  client_send(target, banned_msg, strlen(banned_msg) + 1);

  char success_msg[target->user_len + 20];
  sprintf(success_msg, "%s has been banned", target->user);
  client_send(c, success_msg, strlen(success_msg) + 1);

  target->kicked = true;
  client_kill(target);
}

void cmd_banip(client_t *c, msg_t *m) {
  if (!c->is_admin || !config_get_bool("admin/enable_banip")) {
    char *denied_msg = "You don't have access to this command";
    client_send(c, denied_msg, strlen(denied_msg) + 1);
    return;
  }

  client_t *target = clist_get(m->arg);
  if (NULL == target) {
    char *notfound_msg = "User not found";
    client_send(c, notfound_msg, strlen(notfound_msg) + 1);
    return;
  }

  if (target->is_admin) {
    char *perm_msg = "You cannot ban another admin";
    client_send(c, perm_msg, strlen(perm_msg) + 1);
    info("%s tried to ban another admin (%s)", c->user, target->user);
    return;
  }

  banlist_add(target->ipstr, NULL);
  clist_foreach(cmd_banip_client, c);
}

void cmd_kick(client_t *c, msg_t *m) {
  if (!c->is_admin || !config_get_bool("admin/enable_kick")) {
    char *denied_msg = "You don't have access to this command";
    client_send(c, denied_msg, strlen(denied_msg) + 1);
    return;
  }

  client_t *target = clist_get(m->arg);
  if (NULL == target) {
    char *notfound_msg = "User not found";
    client_send(c, notfound_msg, strlen(notfound_msg) + 1);
    return;
  }

  if (target->is_admin) {
    char *perm_msg = "You cannot kick another admin";
    client_send(c, perm_msg, strlen(perm_msg) + 1);
    info("%s tried to kick another admin (%s)", c->user, target->user);
    return;
  }

  target->kicked = true;

  char *banned_msg  = "You have been kicked!";
  char *success_msg = "User has been kicked";

  info("%s was kicked by %s", target->user, c->user);
  client_send(target, banned_msg, strlen(banned_msg) + 1);
  client_send(c, success_msg, strlen(success_msg) + 1);
  client_kill(target);
}

cmd_t map[] = {
    {.name = "SND", .func = cmd_send,      .arg_required = true },
    {.name = "LST", .func = cmd_list,      .arg_required = false},
    {.name = "HST", .func = cmd_history,   .arg_required = false},
    {.name = "ADL", .func = cmd_admin_log, .arg_required = true },
    {.name = "BAN", .func = cmd_ban,       .arg_required = true },
    {.name = "BIP", .func = cmd_banip,     .arg_required = true },
    {.name = "KCK", .func = cmd_kick,      .arg_required = true },
};

int cmd_data_callback(ssh_session session, ssh_channel channel, void *buffer, uint32_t len, int is_stderr, void *data) {
  client_t *c = (client_t *)data;

  char *text = (char *)buffer;
  if (len < 3 || len > 9000 || text[len - 1] != '\0') {
    debug("(%s@%s) Protocol violation", c->user, c->ipstr, buffer);
    return len;
  }

  msg_t msg;
  msg_load(&msg, text, len);

  for (int i = 0; i < sizeof(map) / sizeof(cmd_t); i++) {
    cmd_t cur = map[i];
    if (!startswith(msg.cmd, cur.name))
      continue;

    if (cur.arg_required && msg.arg_len <= 0)
      continue;

    cur.func(c, &msg);
    goto END;
  }

  debug("(%s@%s) Unknown command", c->user, c->ipstr);

END:
  msg_free(&msg);
  return len;
}
