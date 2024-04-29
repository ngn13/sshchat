// clang-format off

/*

 * sshchat | server
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

#include <errno.h>
#include <libssh/callbacks.h>
#include <libssh/server.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/log.h"
#include "banlist.h"
#include "clist.h"
#include "config.h"
#include "history.h"
#include "session.h"

static ssh_bind server;

void free_all() {
  banlist_free();
  history_free();
  clist_free();
  config_free();
}

void handler(int n) {
  info("Stopping server");

  ssh_bind_free(server);
  ssh_finalize();

  free_all();
  exit(1);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handler);

  char *cfgpath = "/etc/sshchat/config.ini";
  if (argc >= 2)
    cfgpath = argv[1];

  if (!config_load(cfgpath)) {
    config_free();
    return EXIT_FAILURE;
  }

  char *host = config_get_str("general/host");
  char *key  = config_get_str("general/key");
  int port   = config_get_int("general/port");

  info(FG_BOLD "======> sshchat server %s (https://github.com/ngn13/sshchat) <======", VERSION);
  info("Running with configuration file: %s", cfgpath);
  info("Starting server on %s:%d", host, port);

  char banner[strlen("sshchat") + strlen(VERSION)];
  sprintf(banner, "%s_sshchat", VERSION);

  server = ssh_bind_new();
  ssh_bind_options_set(server, SSH_BIND_OPTIONS_BINDADDR, host);
  ssh_bind_options_set(server, SSH_BIND_OPTIONS_RSAKEY, key);
  ssh_bind_options_set(server, SSH_BIND_OPTIONS_BANNER, banner);
  ssh_bind_options_set(server, SSH_BIND_OPTIONS_BINDPORT, &port);

  if (ssh_bind_listen(server) < 0) {
    error("Failed to start the server: %s", ssh_get_error(server));
    config_free();
    return EXIT_FAILURE;
  }

  clist_init();

  while (true) {
    ssh_session session = ssh_new();
    if (ssh_bind_accept(server, session) == SSH_ERROR) {
      error("Failed to accept connection: %s", ssh_get_error(server));
      continue;
    }

    char ipaddr[INET6_ADDRSTRLEN];
    session_getip(session, ipaddr);
    debug("New connection from %s", ipaddr);

    client_t *c = client_new(server, session);
    c->ipstr    = strdup(ipaddr);

    pthread_t t;
    pthread_create(&t, NULL, session_handle, c);
    pthread_detach(t);
  }

  ssh_bind_free(server);
  ssh_finalize();

  free_all();
  return EXIT_SUCCESS;
}
