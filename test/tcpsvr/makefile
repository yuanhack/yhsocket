CC=gcc

CFLAGS = -g -Wall -I../../
LIB=./
DEMO_MACRO= $(CFLAGS) -Wl,-R,$(LIB) -L$(LIB)


##################################################
tcp_demo: update svr1 clt1 svr0 clt0

svr1: svr1.c 
	$(CC) $< $(DEMO_MACRO) -lyhsocket -o $@
clt1: clt1.c 
	$(CC) $< $(DEMO_MACRO) -lyhsocket -o $@

svr0: svr0.c 
	$(CC) $< $(DEMO_MACRO) -lyhsocket -o $@
clt0: clt0.c 
	$(CC) $< $(DEMO_MACRO) -lyhsocket -o $@

##################################################
udp_demo:
	@echo -n # is null 	

##################################################

clean: 
	rm -rf *.o *.so svr[0-9] clt[0-9] test yhsocket.h

update:
	@cd ../../ &&  make &&  cd - && \
		cp -v ../../lib/libyhsocket.so . && \
		cp -v ../../yhsocket.h . && \
		cp -v ../../yhprotocol.h .
