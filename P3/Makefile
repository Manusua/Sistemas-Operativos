######################################################
CC = gcc
CFLAGS = -Wall
EXE =  ejercicio4_main ejercicio4_A ejercicio4_B ejercicio4_C
LIB = -lrt -pthread
######################################################

all : $(EXE)

$(EXE) : % : %.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LIB)

clean:
	rm -f core $(EXE)
