#
# Makefile for tcp_echo
#
MYLIBDIR=mynet
MYLIB=-lmynet -lcurses
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: client_curses

client_curses: client_curses.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o client_curses *~
