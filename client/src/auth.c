#include "auth.h"

#include <libssh/libssh.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "input.h"
#include "term.h"

bool auth_check() {
  int ret = ssh_userauth_password(client.session, client.user, "default");
  if (ret == SSH_AUTH_SUCCESS)
    return true;

  else if (ret == SSH_AUTH_PARTIAL) {
    term_error("Server is refusing connection because your "
               "username is "
               "invalid or it's taken");
    return false;
  }

  return auth_password();
}

bool auth_password() {
  term_print("Server is password protected");

  bool ret     = false;
  char *prompt = input.prompt;
  input_prompt("password");

RETRY:
  if (!ssh_is_connected(client.session)) {
    term_error("Lost connection to the server (you might "
               "have reached the max "
               "retry limit)");
    goto END;
  }

  term_bold("Please enter a password");
  char *pwd = input_get(true);
  int res   = ssh_userauth_password(client.session, client.user, pwd);

  if (res == SSH_AUTH_SUCCESS) {
    term_success("Password is correct");
    ret = true;
    free(pwd);
    goto END;
  }

  else if (res == SSH_AUTH_PARTIAL) {
    term_error("Server is refusing connection because your "
               "username is invalid "
               "or it's taken");
    goto END;
  }

  else
    term_error("Incorrect password, try again");

  free(pwd);
  goto RETRY;

END:
  input_prompt_free();
  input.prompt = prompt;
  return ret;
}
