#
# Makefile for tcp_echo
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}
OBJS=idobata.o idobata_server.o idobata_client.o common.o

all: idobata

idobata: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o idobata *~