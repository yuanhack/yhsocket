CC=gcc

LIB = ../../lib/libsocketyh.so
DEMO_MACRO= -Wall -Wl,-R,$(LIB) -$(LIB)

demo: udps

##################################################
udps: udps

udps: udp_server.c 
	$(CC) $< $(DEMO_MACRO) -lsocketyh -o $@
tcp_client: tcp_client.c 
	$(CC) $< $(DEMO_MACRO) -lsocketyh -o $@
zcin_test: zcin_test.c 
	$(CC) $< $(DEMO_MACRO) -lsocketyh -o $@


##################################################
udp_demo:
	@echo -n # is null 	

##################################################

clean: 
	rm -rf tcp_client tcp_server zcin_test

update:
	cd ../../; make; cd -; cp -v ../../lib/libsocketyh.so .

