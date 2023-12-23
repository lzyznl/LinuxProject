
CC = gcc

LDFLAGS = -lmicrohttpd -lpthread

SRC = server.c
TARGET = server

all:
	$(CC) $(SRC) -o $(TARGET) $(LDFLAGS)


clean:
	rm -f $(TARGET)