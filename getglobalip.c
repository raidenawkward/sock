#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE	512

int port = 80;

void getip(char *url) {
	struct sockaddr_in pin;
	struct hostent *nlp_host;
	int sd = 0;
	int len = 0;
	int i, count = 0;
	int recv_start = 0, recv_end = 0;
	char buf[BUF_SIZE] = { 0 }, myurl[100] = { 0 };
	char host[100] = { 0 }, GET[100] = { 0 }, header[240] =	{ 0 };
	char *pHost = 0;

	///get the host name and the relative address from url name!!!
	strcpy(myurl, url);
	for (pHost = myurl; *pHost != '/' && *pHost != '\0'; ++pHost);
	if ((int)(pHost - myurl) == strlen(myurl))
		strcpy(GET, "/");
	else
		strcpy(GET, pHost);
	*pHost = '\0';
	strcpy(host, myurl);

	///setting socket param
	if ((nlp_host = gethostbyname(host)) == 0) {
		perror("error get host\n");
		exit(1);
	}

	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(INADDR_ANY);
	pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
	pin.sin_port = htons(port);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error opening socket!!!\n");
		exit(1);
	}

	///together the request info that will be sent to web server
	///Note: the blank and enter key byte is necessary,please remember!!!
	strcat(header, "GET");
	strcat(header, " ");
	strcat(header, GET);
	strcat(header, " ");
	strcat(header, "HTTP/1.1\r\n");
	strcat(header, "HOST:");
	strcat(header, host);
	strcat(header, "\r\n");
	strcat(header, "ACCEPT:*/*");
	strcat(header, "\r\nConnection: close\r\n\r\n\r\n");

	///connect to the webserver,send the header,and receive the web sourcecode  
	if (connect(sd, (void *)&pin, sizeof(pin)) == -1)
		printf("error connect to socket\n");

	if (send(sd, header, strlen(header), 0) == -1) {
		perror("error in send \n");
		exit(1);
	}

	///send the message and wait the response!!!
	len = recv(sd, buf, BUF_SIZE, 0);
	if (len < 0)
		printf("receive data error!!!\n");
	else {
		printf("%s", &(buf[157]));
	}
	close(sd);
}

int main() {
	char *url = "www.3322.org/dyndns/getip";
	getip(url);
	return 0;
}
