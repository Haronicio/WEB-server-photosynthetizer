CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -I/usr/include
LDFLAGS=-lmosquitto -lpthread
SRC1=gatewayMQTT 
SRC2=esp-gatewayMQTT

.PHONY: all clean

all: gatewayMQTT esp-gatewayMQTT

gatewayMQTT: gatewayMQTT.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

esp-gatewayMQTT: esp-gatewayMQTT.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f gatewayMQTT esp-gatewayMQTT