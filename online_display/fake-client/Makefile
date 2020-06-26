
CC=gcc
CFLAGS=-I.
DEPS = sockhelp.h



%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
# all: client server
client: fake-client.o sockhelp.o 
	$(CC) -o $@ $^ $(CFLAGS)
# server: fake-server.o sockhelp.o
# 	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -f fake-client.o sockhelp.o 
