#
# Makefile for tcp_echo
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: cmdserver

cmdserver: cmdserver.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o cmdserver *~