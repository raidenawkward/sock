
CFLAGS += -g -lpthread

TGT_GLOBALIP := getglobalip
OBJS_GLOBALIP := getglobalip.o
TGTS += $(TGT_GLOBALIP)

TGT_TCPCLIENT := tcpclient
OBJS_TCPCLIENT := tcpclient.o
TGTS += $(TGT_TCPCLIENT)

TGT_TCPSERVER := tcpserver
OBJS_TCPSERVER := tcpserver.o
TGTS += $(TGT_TCPSERVER)

TGT_UDPCLIENT := udpclient
OBJS_UDPCLIENT := udpclient.o
TGTS += $(TGT_UDPCLIENT)

TGT_UDPSERVER := udpserver
OBJS_UDPSERVER := udpserver.o
TGTS += $(TGT_UDPSERVER)

TGT_FTRANSCLIENT := ftrans_client
OBJS_FTRANSCLIENT := ftrans_client.o
TGTS += $(TGT_FTRANSCLIENT)

TGT_FTRANSSERVER := ftrans_server
OBJS_FTRANSSERVER := ftrans_server.o
TGTS += $(TGT_FTRANSSERVER)

default:
	######################################################
	#	socket testings
	#	usable make argues:
	#	(NULL)	-	show this msg
	#	getglobalip	-	get global ip address
	#	tcpclient
	#	tcpserver
	#	udpclient
	#	udpserver
	#	ftransclient
	#	ftransserver
	#	ftrans	-	both ftrans client and server
	#	clean
	########################################################
getglobalip:$(OBJS_GLOBALIP)
	$(CC) $(CFLAGS) $(OBJS_GLOBALIP) -o $(TGT_GLOBALIP)
tcpclient:$(OBJS_TCPCLIENT)
	$(CC) $(CFLAGS) $(OBJS_TCPCLIENT) -o $(TGT_TCPCLIENT)
tcpserver:$(OBJS_TCPSERVER)
	$(CC) $(CFLAGS) $(OBJS_TCPSERVER) -o $(TGT_TCPSERVER)
udpclient:$(OBJS_UDPCLIENT)
	$(CC) $(CFLAGS) $(OBJS_UDPCLIENT) -o $(TGT_UDPCLIENT)
udpserver:$(OBJS_UDPSERVER)
	$(CC) $(CFLAGS) $(OBJS_UDPSERVER) -o $(TGT_UDPSERVER)
ftransclient:$(OBJS_FTRANSCLIENT)
	$(CC) $(CFLAGS) $(OBJS_FTRANSCLIENT) -o $(TGT_FTRANSCLIENT)
ftransserver:$(OBJS_FTRANSSERVER)
	$(CC) $(CFLAGS) $(OBJS_FTRANSSERVER) -o $(TGT_FTRANSSERVER)
ftrans:$(OBJS_FTRANSSERVER) $(OBJS_FTRANSCLIENT)
	$(CC) $(CFLAGS) $(OBJS_FTRANSCLIENT) -o $(TGT_FTRANSCLIENT)
	$(CC) $(CFLAGS) $(OBJS_FTRANSSERVER) -o $(TGT_FTRANSSERVER)
clean:
	rm -rf *.o $(TGTS)
