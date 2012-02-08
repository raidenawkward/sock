#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "ftrans.h"

#define ETH_NAME "eth0"

int get_local_sin_addr(struct in_addr *sinaddr) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		goto err;

	struct sockaddr_in addr;

	struct ifreq ifr;
	strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		goto err;
	}

	memcpy(&addr, &ifr.ifr_addr, sizeof(struct sockaddr_in));
	memcpy(sinaddr,&(addr.sin_addr), sizeof(struct in_addr));

	close(sock);
	return 0;

err:
	if (sock > 0)
		close(sock);

	return -1;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		mkdir(FTRANS_UPLOAD_PATH, FTRANS_UPLOAD_MOD);
	} else {
		mkdir(argv[1], FTRANS_UPLOAD_MOD);
	}

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(FTRANS_PORT);
	if (get_local_sin_addr(&addr_server.sin_addr) < 0) {
		printf("error when getting local inet info\n");
		goto err;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("failed to create socket descripter");
		goto err;
	}

	if (bind(sockfd, (struct sockaddr*)&addr_server, sizeof(struct sockaddr_in)) < 0) {
		perror("failed to bind");
		goto err;
	}

	if (listen(sockfd, FTRANS_TCP_LISTENBACKLOG) < 0) {
		perror("failed to listen");
		goto err;
	}

	while (1) {
		socklen_t socklen_client = 0;
		struct sockaddr_in addr_client;

		int sockfd_client = accept(sockfd, (struct sockaddr*)&addr_client, &socklen_client);
		if (sockfd_client < 0) {
			perror("error client connection detacted");
			continue;
		}

	}

done:
	return 0;
err:
	if (sockfd > 0)
		close(sockfd);
	return -1;
}
