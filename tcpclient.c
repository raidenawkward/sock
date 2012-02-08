#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP "192.168.3.13"
#define PORT 1234

#define BUF_LENGTH (1024)
char buf[BUF_LENGTH];

int main(int argc, char** argv) {
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0) {
		printf("error when getting sockfd\n");
		return -1;
	}

	struct sockaddr_in addr_dest;
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_port = htons(PORT);
	addr_dest.sin_addr.s_addr = inet_addr(IP);

	if (connect(sockfd,(struct sockaddr*)&addr_dest,sizeof(struct sockaddr_in)) != 0) {
		printf("error when connecting\n");
		return -1;
	}

	strcpy(buf,argc > 1? argv[1] : "");
	if (send(sockfd,(void*)buf,sizeof(buf),0) < 0)
		printf("error when sending msg\n");

	memset(buf,0x00,sizeof(buf));
	if (recv(sockfd,(void*)buf,sizeof(buf),0) < 0)
		printf("error when recving msg\n");
	else {
		printf("message received:\n");
		printf("%s\n",buf);
	}

	close(sockfd);
	return 0;
}
