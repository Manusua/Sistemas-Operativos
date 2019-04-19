TARGET = .

CC = gcc
CFLAGS = -g -Wall -pthread -I.
LDLIBS = -lrt -lncurses

BOLD=\e[1m
NC=\e[0m

all: simulador monitor

clean:
	rm -r -f
	rm simulador
	rm monitor

simulador:
	$(CC) $(CFLAGS) mapa.c simulador.c -o simulador -lrt -lm

monitor:
	$(CC) $(CFLAGS) gamescreen.c mapa.c monitor.c -o monitor -lrt -lncurses -lm
