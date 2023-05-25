CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic -std=c11 -I/usr/include
LDFLAGS=-lmosquitto -lpthread
SRC1=gatewayMQTT 

.PHONY: all clean

all: gatewayMQTT

gatewayMQTT: gatewayMQTT.c
	$(CC) $(CFLAGS) -o $@ $< cJSON.c jsonparser.c $(LDFLAGS)

clean:
	rm -f gatewayMQTT esp-gatewayMQTT