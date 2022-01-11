YFLAGS = -d					# force creation of y.tab.h
SOURCES	=	hoc.y \
		hoc.h \
		code.c \
		init.c \
		math.c \
		symbol.c

OBJS = hoc.o code.o init.o math.o symbol.o	# abbreviation

PRINT	=	print
TOUCH	=	touch

hoc4:	$(OBJS)
	cc $(OBJS) -lm -o hoc4

hoc.o code.o init.o symbol.o:	hoc.h

code.o init.o symbol.o:	x.tab.h

x.tab.h:	y.tab.h
	-cmp -s x.tab.h y.tab.h || cp y.tab.h x.tab.h

print:	makefile	$(SOURCES)
	$(PRINT)	$?
	$(TOUCH)	$@

clean:
	rm -f $(OBJS) [xy].tab.[ch]
