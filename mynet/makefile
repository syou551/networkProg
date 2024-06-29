#
# Makefile for libmynet
#
OBJS = init_tcpserver.o init_tcpclient.o other.o init_udpserver.o init_udpclient.o init_tcpclient_ip.o init_udpclient_broadcast.o
AR = ar -qc

libmynet.a : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

${OBJS}: mynet.h

clean:
	${RM} *.o
