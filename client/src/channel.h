#pragma once
#include <libssh/libssh.h>

void channel_send(char *);
void *channel_sender(void *);
int channel_receiver(ssh_session, ssh_channel, void *, uint32_t, int, void *);
