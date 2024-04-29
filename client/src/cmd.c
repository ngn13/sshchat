#include "cmd.h"

#include <stdlib.h>
#include <string.h>

#include "../../lib/util.h"
#include "channel.h"
#include "client.h"
#include "term.h"

int cmd_disconnect(args_t *cmd) {
  term_print("Closing connection to the server");
  return RET_BREAK;
}

int cmd_list(args_t *cmd) {
  channel_send("LST");
  return RET_CONTINUE;
}

int cmd_hist(args_t *cmd) {
  channel_send("HST");
  return RET_CONTINUE;
}

int cmd_admin(args_t *cmd) {
  char adl_cmd[cmd->data_len + 4];
  sprintf(adl_cmd, "ADL %s", cmd->data);
  channel_send(adl_cmd);
  return RET_CONTINUE;
}

int cmd_ban(args_t *cmd) {
  char adl_cmd[cmd->data_len + 4];
  sprintf(adl_cmd, "BAN %s", cmd->data);
  channel_send(adl_cmd);
  return RET_CONTINUE;
}

int cmd_banip(args_t *cmd) {
  char adl_cmd[cmd->data_len + 4];
  sprintf(adl_cmd, "BIP %s", cmd->data);
  channel_send(adl_cmd);
  return RET_CONTINUE;
}

int cmd_kick(args_t *cmd) {
  char adl_cmd[cmd->data_len + 4];
  sprintf(adl_cmd, "KCK %s", cmd->data);
  channel_send(adl_cmd);
  return RET_CONTINUE;
}

int cmd_clear(args_t *cmd) {
  term_clear();
  return RET_CONTINUE;
}

cmd_t map[] = {
    {.name = ".exit",    .func = cmd_disconnect, .arg_required = false},
    {.name = ".help",    .func = cmd_help,       .arg_required = false},
    {.name = ".list",    .func = cmd_list,       .arg_required = false},
    {.name = ".history", .func = cmd_hist,       .arg_required = false},
    {.name = ".admin",   .func = cmd_admin,      .arg_required = true },
    {.name = ".ban",     .func = cmd_ban,        .arg_required = true },
    {.name = ".banip",   .func = cmd_banip,      .arg_required = true },
    {.name = ".kick",    .func = cmd_kick,       .arg_required = true },
    {.name = ".clear",   .func = cmd_clear,      .arg_required = false},
};

int cmd_run(char *cmd_char) {
  args_t args;
  args_parse(&args, cmd_char);
  int ret = RET_INVALID;

  for (int i = 0; i < sizeof(map) / sizeof(cmd_t); i++) {
    if (!eq(map[i].name, args.base))
      continue;

    if (!args.has_data && map[i].arg_required) {
      term_error("%s command requires and argument", map[i].name);
      return RET_CONTINUE;
    }

    ret = map[i].func(&args);
    break;
  }

  args_free(&args);
  return ret;
}

int cmd_help(args_t *cmd) {
  term_bold("You are on sshchat version %s", VERSION);
  term_print("Avaliable commands are:");

  for (int i = 0; i < sizeof(map) / sizeof(cmd_t); i++)
    term_bold("\t%s", map[i].name);

  term_print("For more information checkout the documentation");
  return RET_CONTINUE;
}
