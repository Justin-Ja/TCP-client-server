CC = gcc
CFLAGS = -std=c99 -Wall -pedantic

all: clientTCP serverTCP

clientTCP: clientTCP.c
	$(CC) $(CFLAGS) -o clientTCP clientTCP.c

serverTCP: serverTCP.c
	$(CC) $(CFLAGS) -o serverTCP serverTCP.c

clean:
	rm -f clientTCP serverTCP
