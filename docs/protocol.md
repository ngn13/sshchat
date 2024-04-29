# Authentication
When a client connects to a server, it attempts to authenticate using a random password.
Server does one of the following:
- Returns with `SSH_AUTH_PARTIAL` if the username is invalid (contains non-letter and non-number chars or longer then the max username length or it's taken)
- Returns `SSH_AUTH_ERROR` if the server is password protected
- Returns `SSH_AUTH_SUCCESS` if the server is not password protected

Depending on the response, client does one of the following:
- If it receives `SSH_AUTH_PARTIAL`, tells user the username is invalid
- If it receives `SSH_AUTH_ERROR`, then it asks the user for a password
- If it receives `SSH_AUTH_SUCCESS`, attempts to create a communication channel

# Channels
All the communication is handled on a single channel (no PTY allocation, no shell requests).
The client sends the commands on this channel, and the server sends the messages that it wants
the client to display. It's simple as that.

# Commands
All the commands are 3 bytes long, and may require a single argument. If a command Argument is specified
requires an argument, it should be specified right after 
right after the command, 

Server does not have specific response for the commands, it just have a list of things it should do
when a command is received.

### `SND <message>`
Send command. When the server receives this command, it should send the specified
message to all the clients.

### `LST`
List command. When this command is received, server should return a list of all the
connected clients. Or (as this is the case with the current implementation) it should tell the
client that it's alone.

### `HST`
History command. Server should return all the messages it has stored (in order).
Current allows the server owner to configure the history size.

### `ADL <password>`
Admin login command. If admin authentication is disabled, server should tell the client that,
it's disabled, if not, the server should verify the password and authenticate the client as
admin. It should also inform the client ("hey you are now an admin yeay!!").

### `KICK <username>`
Kick command. In order for the server to kick the specified user (target):
- User should be authenticated as admin
- Target should be online 
- Target should not be authenticated as admin (admins can't ban other admins)

If all requirements are satisfied, server will close the SSH channel and the session of the target.
The target will be able to rejoin.

### `BAN <username>`
Ban command. In order for the server to ban the specified user (target):
- User should be authenticated as admin
- Target should be online 
- Target should not be authenticated as admin (admins can't ban other admins)

If all requirements are satisfied, server will close the SSH channel and the session of the target
When the target is banned, it will still be able to authenticate and create a channel, however target
will be kicked immediately after the channel creation. 

Note that this ban is nick based, so it can easily be evaded by changing username.

### `BIP <username>`
Ban IP command. In order for the server to ban the specified user (target):
- User should be authenticated as admin
- Target should be online 
- Target should not be authenticated as admin (admins can't ban other admins)

If all requirements are satisfied, server will close the SSH channel and the session of the target
When the target is banned, it will still be able to authenticate and create a channel, however target
will be kicked immediately after the channel creation. 

This ban is IP based, so target cannot easily avoid it by changing their username.

# Disconnecting
Connection is terminated by closing the channel, and then the SSH session. Connection can also be terminated
in the authentication phase, if the client exceeds the password retry limit.
