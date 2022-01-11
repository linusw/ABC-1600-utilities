all:	siv install

siv: f0.o f1.o f2.o f3.o f4.o f5.o f6.o f7.o f8.o f9.o f10.o f12.o 
	cc -n f0.o f1.o f2.o f3.o f4.o f5.o f6.o f7.o f8.o f9.o f10.o f12.o\
 -ltermlib -o siv

f0.o f1.o f2.o f3.o f4.o f5.o f6.o f7.o f8.o f9.o f10.o f12.o:\
 define.h struct.h
f2.o: term.c
f0.o: global.h

install:	
#	cp siv /bin/siv
#	strip  /bin/siv
