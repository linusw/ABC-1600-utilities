#
# makefile for paint
#

SOURCES = windows.c cmd1.c cmd2.c cmd3.c others.c

OBJECTS = windows.o cmd1.o cmd2.o cmd3.o others.o

CFLAGS = -O -n

paint : $(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) $(LIBES) -o paint

all: paint pdriver converter

pdriver: pdriver.c
	cc -O pdriver.c -o pdriver

converter: converter.c
	cc -O converter.c -o converter
