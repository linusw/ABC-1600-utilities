systest1600: main.o tests.o cio.o batclk.o dlymicr.o dma.o nvram.o scc.o memory.o print.o keyb.o mouse.o mover.o bw.o cross.o lprintf.o winch.o floppy.o
	cc -O -s tests.o main.o cio.o batclk.o dlymicr.o dma.o nvram.o scc.o memory.o print.o keyb.o mouse.o mover.o bw.o cross.o lprintf.o winch.o floppy.o -o systest1600
	tar cvf /dev/mf2 systest1600
main.o: ../main.c
	cc -O -c ../main.c
tests.o: ../tests.c
	cc -O -c ../tests.c
cio.o: ../cio.c 
	cc -O -c ../cio.c 
batclk.o : ../batclk.c 
	cc -O -c ../batclk.c 
dlymicr.o : ../dlymicr.s
	as ../dlymicr.s
dma.o : ../dma.c 
	cc -O -c ../dma.c 
nvram.o : ../nvram.c
	cc -O -c ../nvram.c
scc.o: ../scc.c
	cc -O -c ../scc.c
memory.o: ../memory.c
	cc -O -c ../memory.c
print.o: ../print.c
	cc -O -c ../print.c
keyb.o: ../keyb.c
	cc -O -c ../keyb.c
mouse.o: ../mouse.c
	cc -O -c ../mouse.c
mover.o: ../mover.c
	cc -O -c ../mover.c
bw.o: ../bw.c
	cc -O -c ../bw.c
cross.o: ../cross.c
	cc -O -c ../cross.c
lprintf.o: ../lprintf.c
	cc -O -c ../lprintf.c
winch.o: ../winch.c
	cc -O -c ../winch.c
floppy.o: ../floppy.c
	cc -O -c ../floppy.c
