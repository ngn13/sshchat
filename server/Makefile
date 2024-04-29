LIB_HEADERS = $(wildcard ../lib/*.h)
LIB_CSRCS   = $(wildcard ../lib/*.c)
HEADERS = $(wildcard src/*.h)
CSRCS   = $(wildcard src/*.c)
CFLAGS  = -O3 -fstack-clash-protection -fstack-protector-strong -pie -fPIE
VERSION = 1.0
prefix  = /usr
CC      = gcc

sshchat-server: $(CSRCS) $(HEADERS) $(LIB_CSRCS) $(LIB_HEADERS)
	$(CC) $(CFLAGS) $(CSRCS) $(LIB_CSRCS) -o $@ -DVERSION=\"${VERSION}\" -lpthread -lssh -linih

install:
	mkdir -pv $(DESTDIR)/etc/sshchat
	install -v -Dm600 install/config.ini $(DESTDIR)/etc/sshchat
	cp -r install/keys $(DESTDIR)/etc/sshchat
	chown root:root -R $(DESTDIR)/etc/sshchat
	install -v -m755 sshchat-server $(DESTDIR)$(prefix)/bin/sshchat-server

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/sshchat-server

format:
	clang-format -i -style=file ../lib/*.c ../lib/*.h ./src/*.c ./src/*.h

keygen:
	rm -rf install/keys
	mkdir -pv install/keys
	ssh-keygen -t rsa -f install/keys/key.rsa -N ""

.PHONY: install uninstall format keygen
