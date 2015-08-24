#########################################################################
#      This make file has been adapted from the potato program          #
#########################################################################

all: main

CC = gcc
CFLAGS = -DTESTING_OUTPUT -g -O2

#Linker Options ; use lrt for sem_init(3) posix interfaces..
LD = gcc
LDFLAGS = -g -lrt

# compiling the source file.
bathroom.o: bathroom.c defs.h
	$(CC) $(CFLAGS) -c bathroom.c
enter.o: enter.c defs.h
	$(CC) $(CFLAGS) -c enter.c

# linking the program.
main: bathroom.o enter.o
	$(LD) $(LDFLAGS) bathroom.o -o bathroom 
	$(LD) $(LDFLAGS) enter.o -o enter 

# cleaning everything that can be automatically recreated with "make".
clean:
	/bin/rm -f bathroom bathroom.o enter enter.o skadama.tmp

