CC=gcc
CFLAGS=-I. -std=c99
LIBS=-lpthread

all: server client

server:
	$(CC) -o server server.c $(CFLAGS) $(LIBS)

client:
	$(CC) -o client client.c $(CFLAGS) $(LIBS)

clean:
	rm -f server client
