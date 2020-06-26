
CC=gcc
CFLAGS=-I.




%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
all: udp_client udp_server
client: udp_client.o 
	$(CC) -o $@ $^ $(CFLAGS)
server: udp_server.o
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -f udp_client.o udp_server.o  
