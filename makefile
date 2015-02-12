CC=gcc


CFLAGS = -g -Wall

SO_MACRO= -shared -fpic
DEST=libyhsocket.so

SRC=yhsocket.c yhprotocol.c
HED=yhsocket.h yhprotocol.h


$(DEST): $(SRC) $(HED)
	$(CC) $(CFLAGS) $(SO_MACRO) -o $@ $^ \
		&& mkdir -p lib \
		&& cp $(DEST) yh*.h lib 
clean:
	rm -rf $(DEST) lib


