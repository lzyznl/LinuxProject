CC=gcc
CFLAGS=-I. -Wall -std=c99
LIBS=-ljson-c -lpthread

all: server client

server: server.c
	$(CC) -o server server.c $(CFLAGS) $(LIBS)

client: client.c
	$(CC) -o client client.c $(CFLAGS) $(LIBS)

clean:
	rm -f server client
