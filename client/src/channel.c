#include <libssh/libssh.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/log.h"
#include "../../lib/util.h"
#include "channel.h"
#include "client.h"
#include "cmd.h"
#include "input.h"
#include "term.h"

int channel_command(char *cmd) {
#define MATCH(s) eq(cmd, s)

  int ret = cmd_run(cmd);
  if (ret != RET_INVALID)
    return ret;

  if (cmd[0] == '.' && strlen(cmd) >= 2 && cmd[1] != ' ') {
    term_error("Unknown command: %s", cmd);
    return RET_CONTINUE;
  }

  return ret;
}

void channel_send(char *msg) {
  ssh_channel_write(client.channel, msg, strlen(msg) + 1);
}

bool channel_check() {
  if (client_is_alive())
    return true;

  if (!client.quit) {
    term_error("Lost connection to the server");
    input_pause();
  }

  return false;
}

void *channel_sender(void *data) {
  size_t input_len, cmd_len;
  char *input;
  int ret;

  while (channel_check()) {
    input = input_get(false);
    if (NULL == input)
      break;

    input_len = strlen(input);

    if (!channel_check())
      break;

    if (input_len == 0) {
      free(input);
      continue;
    }

    ret = channel_command(input);
    if (RET_CONTINUE == ret) {
      free(input);
      continue;
    }

    else if (RET_BREAK == ret) {
      free(input);
      break;
    }

    char cmd[input_len + 5];
    bzero(cmd, input_len + 5);

    sprintf(cmd, "SND %s", input);
    channel_send(cmd);
    free(input);
  }

  pthread_mutex_lock(&client.lock);
  client.quit = true;
  pthread_mutex_unlock(&client.lock);
  return NULL;
}

int channel_receiver(ssh_session session, ssh_channel channel, void *buffer, uint32_t len, int is_stderr, void *data) {
  char *str = (char *)buffer;
  if (str[len - 1] != '\0')
    return len;

  args_t *args = (args_t *)data;
  int i        = 0;
  term_timenl();

  if (len <= 3 || str[i] != '<')
    goto SKIP_BOLD;

  for (int i = 0; i < len; i++) {
    if (str[i] == '>')
      goto PRINT_BOLD;
  }

  goto SKIP_BOLD;

PRINT_BOLD:
  for (; i < len; i++) {
    if (i != 0 && str[i - 1] == '>' && str[i] == ' ')
      break;
    term_boldnl("%c", str[i]);
  }

SKIP_BOLD:
  for (; i < len; i++)
    term_printnl("%c", str[i]);
  term_printnl("\n");

  return len;
}
