#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#define PORT 1234
#define IP_ADDRESS "192.168.3.13"
#define BUF_SIZE 1024

int main() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);	
	if (sockfd < 0)
		return -1;

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = PORT;
	addr_server.sin_addr.s_addr = inet_addr(IP_ADDRESS);

	if (bind(sockfd, (struct sockaddr*)&addr_server, sizeof(struct sockaddr_in)) < 0) {
		printf("failed to bind\n");
		return -1;
	}

	char buf[BUF_SIZE];
	memset(buf, 0x00, BUF_SIZE);

	while (1) {
		socklen_t sock_length = 0;
		struct sockaddr_in addr_client;

		if (recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&addr_client, &sock_length) < 0) {
			printf("failed to recv\n");
			continue;
		} else {
			printf("msg recved: %s\n",buf);
			memset(buf, 0x00, BUF_SIZE);
		}
	}

	close(sockfd);
	return 0;
}
