LIB_HEADERS = $(wildcard ../lib/*.h)
LIB_CSRCS   = $(wildcard ../lib/*.c)
HEADERS = $(wildcard src/*.h)
CSRCS   = $(wildcard src/*.c)
CFLAGS  = -O3 -fstack-clash-protection -fstack-protector-strong -pie -fPIE
VERSION = 1.0
prefix  = /usr
CC      = gcc

sshchat: $(CSRCS) $(HEADERS) $(LIB_CSRCS) $(LIB_HEADERS)
	$(CC) $(CFLAGS) $(CSRCS) $(LIB_CSRCS) -o $@ -DVERSION=\"${VERSION}\" -lpthread -lcurses -lssh

install:
	install -v -m755 sshchat $(DESTDIR)$(prefix)/bin/sshchat

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/sshchat

format:
	clang-format -i -style=file ../lib/*.c ../lib/*.h ./src/*.c ./src/*.h

.PHONY: install uninstall format
