#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUF_LENGTH (1024)
#define PORT 1234
#define IP "192.168.3.13"

int main(int argc, char** argv) {
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0) {
		printf("error when getting sockfd\n");
		return -1;
	}

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(PORT);
	addr_server.sin_addr.s_addr = inet_addr(IP);

	if (bind(sockfd,(struct sockaddr*)&addr_server,sizeof(struct sockaddr)) < 0) {
		printf("error when binding\n");
		return -1;
	}

	if (listen(sockfd,5) < 0) {
		printf("error when listenning\n");
		return -1;
	}

	while (1) {
		socklen_t socket_length;
		int clientfd = accept(sockfd,(struct sockaddr*)&addr_server,&socket_length);
		if (clientfd < 0) {
			printf("error when dealing with connection\n");
			continue;
		} else {
			printf("connection gotten\n");
		}

		char buf_recv[BUF_LENGTH];
		read(clientfd,buf_recv,sizeof(buf_recv));
		printf("msg recved : %s\n",buf_recv);
		close(clientfd);
	}

	return 0;
}

