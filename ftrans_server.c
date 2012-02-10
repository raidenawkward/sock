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

#include <pthread.h>

#include "ftrans.h"

#define ETH_NAME "eth0"

pid_t FORKS[FTRANS_MAX_FORKS];
int CURRENT_FORKS = 0;

int get_fork_index(pid_t pid) {
	if (pid < 0)
		return -1;

	int i;
	for (i = 0; i < FTRANS_MAX_FORKS; ++i) {
		if (FORKS[i] == pid)
			return i;
	}

	return -1;
}

int get_next_fork_index() {
	int i;
	for (i = 0; i < FTRANS_MAX_FORKS; ++i) {
		if (FORKS[i] < 0)
			return i;
	}

	return -1;
}

void* forks_listener(void* arg) {
	while(1) {
		if (CURRENT_FORKS) {
			pid_t term_id = wait(NULL);
			int index = get_fork_index(term_id);
			if (index < 0)
				continue;
			FORKS[index] = -1;
			--CURRENT_FORKS;
		}
	}
}

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

int get_received_file_name(int socketClient, char* name) {
	if (socketClient < 0 || !name)
		return -1;

	return sprintf(name,"%d",socketClient);
}

int receive_file_from_client_socket(int socketClient, const char* pathToSave) {
	if (socketClient < 0 || !pathToSave)
		return -1;
	FILE* fp = fopen(pathToSave, "wb");
	if (!fp)
		return -1;

	char buf[FTRANS_RECV_BUF_SIZE];
	size_t sz_read = 0;
	size_t sz_total_read = 0;
	while(sz_read = recv(socketClient, buf, FTRANS_RECV_BUF_SIZE, 0)) {
		if (!sz_read)
			break;
		sz_total_read += sz_read;

		if (fwrite(buf, sz_read, 1, fp) != sz_read) {
			goto err;
		}
	}

	fclose(fp);
	close(socketClient);

	return sz_total_read;

err:
	if (fp)
		fclose(fp);
	if (socketClient > 0)
		close(socketClient);
	return -1;
}

int main(int argc, char** argv) {
	char path_to_save[1024];
	memset(path_to_save, 0x00, 1024);

	if (argc < 2) {
		mkdir(FTRANS_UPLOAD_PATH, FTRANS_UPLOAD_MOD);
		strcpy(path_to_save, FTRANS_UPLOAD_PATH);
	} else {
		mkdir(argv[1], FTRANS_UPLOAD_MOD);
		strcpy(path_to_save, argv[1]);
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

	int i;
	for(i = 0; i < FTRANS_MAX_FORKS; ++i) {
		FORKS[i] = -1;
	}

	pthread_t thread_listener;
	if (pthread_create(&thread_listener, NULL, forks_listener, NULL) != 0) {
		perror("failed to create listener thread");
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

		char name[256];
		memset(name, 0x00, 256);
		if (get_received_file_name(sockfd_client, name) < 0) {
			continue;
		}
		char path[1024];
		memset(path,0x00, 1024);
		strcat(path, path_to_save);
		strcat(path, "/");
		strcat(path, name);

		int new_pid_index = get_next_fork_index();
		FORKS[new_pid_index] = fork();
		++CURRENT_FORKS;

		if (FORKS[new_pid_index] < 0) {
			perror("failed to fork");
			FORKS[new_pid_index] = -1;
			--CURRENT_FORKS;
		} else {
			printf("saving file to %s\n",path);
			receive_file_from_client_socket(sockfd_client, path);
			close(sockfd_client);
		}
	}

done:
	return 0;
err:
	if (sockfd > 0)
		close(sockfd);
	return -1;
}
