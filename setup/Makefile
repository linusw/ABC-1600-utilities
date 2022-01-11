#
# Makefile for setup
#

SOURCES =       setup.c \
		setupinv.c \
		setup.h

OBJECTS =       setup.o
OBJECTS1 =      setupinv.o

CFLAGS = -O -n
CC = cc

all: setup setupinv

setup: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

setup.o: setup.c setup.h
	$(CC) -c $(CFLAGS) setup.c

setupinv: $(OBJECTS1)
	$(CC) $(CFLAGS) $(OBJECTS1) -o $@

setupinv.o: setupinv.c
	$(CC) -c $(CFLAGS) setupinv.c

#
# Print all recently changed files.
#

print: Makefile $(SOURCES)
	print $?
	touch $@
