
SOURCES := $(shell ls *.c)
OBJS := $(SOURCES:.c=.o)
CFLAGS += -g

OBJS_GLOBALIP := getglobalip.o
OBJS_TCPCLIENT := tcpclient.o
OBJS_TCPSERVER := tcpserver.o
OBJS_UDPCLIENT := udpclient.o
OBJS_UDPSERVER := udpserver.o

default:$(OBJS)
getglobalip:$(OBJS_GLOBALIP)
	$(CC) $(CFLAGS) $(OBJS_GLOBALIP) -o getglobalip
tcpclient:$(OBJS_TCPCLIENT)
	$(CC) $(CFLAGS) $(OBJS_TCPCLIENT) -o tcpclient
tcpserver:$(OBJS_TCPSERVER)
	$(CC) $(CFLAGS) $(OBJS_TCPSERVER) -o tcpserver
udpclient:$(OBJS_UDPCLIENT)
	$(CC) $(CFLAGS) $(OBJS_UDPCLIENT) -o udpclient
udpserver:$(OBJS_UDPSERVER)
	$(CC) $(CFLAGS) $(OBJS_UDPSERVER) -o udpserver

clean:
	rm -rf *.o
