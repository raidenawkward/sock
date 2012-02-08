#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 1234
#define IP_ADDRESS "192.168.3.13"
#define BUF_SIZE 1024

int main(int argc, char** argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return -1;

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = PORT;
	addr_server.sin_addr.s_addr = inet_addr(IP_ADDRESS);

	char buf[BUF_SIZE];
	memset(buf,0x00,BUF_SIZE);
	strcpy(buf,argc > 1? argv[1] : "");

	if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&addr_server, sizeof(struct sockaddr_in)) < 0) {
		printf("failed to send msg\n");
		return -1;
	}

	close(sockfd);
	return 0;
}
