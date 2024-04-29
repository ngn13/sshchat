// clang-format off

/*

 * sshchat | client
 * Written by ngn (https://ngn.tf) (2024)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

// clang-format on

#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/log.h"
#include "../../lib/util.h"
#include "auth.h"
#include "channel.h"
#include "client.h"
#include "input.h"
#include "term.h"

void handler(int n) {
  if (NULL != client.channel)
    return term_print("Enter \".exit\" to quit");

  term_finish();
  client_free();
  exit(0);
}

void help() {
  info("You are running sshchat %s "
       "(https://github.com/ngn13/sshchat)",
      VERSION);
  info("Usage information: " FG_BOLD "sshchat [user@host]");

  printf("\n");
  info("This program is free software: you can "
       "redistribute it and/or modify");
  printf("    it under the terms of the GNU General Public "
         "License as "
         "published by\n");
  printf("    the Free Software Foundation, either version "
         "3 of the License, "
         "or\n");
  printf("    (at your option) any later version.\n");
  printf("\n");
  printf("    This program is distributed in the hope that "
         "it will be useful\n");
  printf("    but WITHOUT ANY WARRANTY; without even the "
         "implied warranty of\n");
  printf("    MERCHANTABILITY or FITNESS FOR A PARTICULAR "
         "PURPOSE.  See the\n");
  printf("    GNU General Public License for more details.\n");
  printf("\n");
  printf("    You should have received a copy of the GNU "
         "General Public "
         "License\n");
  printf("    along with this program.  If not, see "
         "<https://www.gnu.org/licenses/>.\n");
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handler);
  setlocale(LC_ALL, "");

  if (argc != 2) {
    help();
    return EXIT_SUCCESS;
  }

  term_init();
  term_bold("              __         __          __ ");
  term_bold("   __________/ /_  _____/ /_  ____ _/ /_");
  term_bold("  / ___/ ___/ __ \\/ ___/ __ \\/ __ `/ __/");
  term_bold(" (__  |__  ) / / / /__/ / / / /_/ / /_  ");
  term_bold("/____/____/_/ /_/\\___/_/ /_/\\__,_/\\__/  ");
  term_bold("                   Welcome to sshchat %s\n", VERSION);
  input_prompt("not connected");

  if (!client_init(argv[1])) {
    input_pause();
    return EXIT_FAILURE;
  }

  term_print("Connecting to %s:%d...", client.ip, client.port);
  client.session = ssh_new();
  if (NULL == client.session) {
    term_error("Failed to create a SSH session");
    input_pause();
    return EXIT_FAILURE;
  }

  ssh_options_set(client.session, SSH_OPTIONS_HOST, client.ip);
  ssh_options_set(client.session, SSH_OPTIONS_PORT, &client.port);

  int ret = ssh_connect(client.session);
  if (ret != SSH_OK) {
    term_error("Failed to connect to %s:%d: %s", client.ip, client.port, ssh_get_error(client.session));
    goto FAIL;
  }

  char *banner = (char *)ssh_get_serverbanner(client.session);
  if (!endswith(banner, "_sshchat")) {
    term_error("Server is not a sshchat server (server banner is %s)", banner);
    goto FAIL;
  }

  if (!endswith(banner, VERSION "_sshchat")) {
    term_bold("Server version mismatch (%s), do you want to continue?", banner);
    if (!input_yn())
      goto FAIL;
  }

  ssh_key key;
  if (ssh_get_server_publickey(client.session, &key) != SSH_OK) {
    term_error("Failed to get the public key of the server");
    goto FAIL;
  }

  unsigned char *hash = NULL;
  size_t hash_len;

  if (ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hash_len) != SSH_OK) {
    term_error("Failed to get the key hash of the server");
    goto FAIL;
  }

  char *fingerprint = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hash_len);
  term_bold("Fingerprint: %s", fingerprint);

  ssh_key_free(key);
  ssh_clean_pubkey_hash(&hash);
  ssh_string_free_char(fingerprint);

  switch (ssh_session_is_known_server(client.session)) {
  case SSH_KNOWN_HOSTS_CHANGED:
    term_error("!!! SERVER FINGERPRINT CHANGED !!!");
    term_error("Potential MiTM attack detected, closing connection");
    goto FAIL;

  case SSH_KNOWN_HOSTS_ERROR:
    term_error("Failed to check known_hosts file, file may be corrupted");
    term_error("Closing connection as its not possible to check the server integrity");
    goto FAIL;

  case SSH_KNOWN_HOSTS_OK:
    break;

  default:
    term_bold("Server is not in the known_hosts file, do you trust it?");
    if (!input_yn()) {
      term_error("Key is not trusted, closing connection");
      goto FAIL;
    }

    ssh_session_update_known_hosts(client.session);
    break;
  }

  if (!auth_check())
    goto FAIL;

  client.channel = ssh_channel_new(client.session);
  if (NULL == client.channel) {
    term_error("Failed to create remote channel: %s", ssh_get_error(client.session));
    goto FAIL;
  }

  ret = ssh_channel_open_session(client.channel);
  if (ret != SSH_OK) {
    term_error("Failed to open the remote channel: %s", ssh_get_error(client.session));
    goto FAIL;
  }

  input_prompt_free();
  input_prompt("%s@%s", client.user, client.ip);
  term_success("Connection is ready!");

  pthread_t sender;
  struct ssh_channel_callbacks_struct channel_callbacks = {
      .userdata              = &client,
      .channel_data_function = channel_receiver,
  };
  ssh_callbacks_init(&channel_callbacks);
  ssh_set_channel_callbacks(client.channel, &channel_callbacks);

  pthread_create(&sender, NULL, channel_sender, NULL);
  ssh_event event = ssh_event_new();
  ssh_event_add_session(event, client.session);

  while (client_is_alive() && !client.quit) {
    if (ssh_event_dopoll(event, 1) == SSH_ERROR)
      break;
  }

  if (input.active && !client.quit) {
    pthread_mutex_lock(&client.lock);
    client.quit = true;
    pthread_mutex_unlock(&client.lock);

    term_error("Lost connection to the server");
    term_bold("Hit any key to exit");
  }

  pthread_join(sender, NULL);
  client_free();
  term_finish();
  return EXIT_SUCCESS;

FAIL:
  input_pause();
  term_finish();

  client_free();
  return EXIT_FAILURE;
}
