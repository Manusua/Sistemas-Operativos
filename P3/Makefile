CPP = gcc
CFLAGS = -g -Wall -pedantic
LDLIBS = -lrt -lpthread

BOLD=\e[1m
NC=\e[0m

PROGS = mq_ejemplo_p1  mq_ejemplo_p2 shm_ejemplo_p1  shm_ejemplo_p2 ejercicio2 ejercicio2_solved ejercicio3_consumidor ejercicio3_productor

all: $(PROGS)

clean:
	@rm -f $(PROGS)

$(PROGS):
	$(CPP) $(CFLAGS) $@.c -o $@ $(LDLIBS)
