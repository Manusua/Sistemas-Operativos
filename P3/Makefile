######################################################
CC = gcc
CFLAGS = -Wall
EXE = ejercicio2 ejercicio2_solved ejercicio4_main ejercicio4_A ejercicio4_B ejercicio4_C
EXECOLA = ejercicio3_consumidor ejercicio3_productor
EXEALL = $(EXE) $(EXECOLA)
LIB = -lrt -pthread
######################################################

all : $(EXEALL)

$(EXECOLA) : % : %.c queue.c
	$(CC) $(CFLAGS) $@.c queue.c -o $@ $(LIB)

$(EXE) : % : %.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LIB)

clean:
	rm -f core $(EXEALL)
