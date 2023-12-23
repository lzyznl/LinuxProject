
CC = gcc

LDFLAGS = -lmicrohttpd -lpthread -std=c99

SRC = server.c
TARGET = server

all:
	$(CC) $(SRC) -o $(TARGET) $(LDFLAGS)


clean:
	rm -f $(TARGET)