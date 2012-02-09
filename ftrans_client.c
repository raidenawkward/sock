#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#include "ftrans.h"

void show_help() {
	printf("usage: client [file] [target ip]\n");
}

size_t load_data_from_file(FILE* fp, char* buf, size_t size) {
	return fread(buf, size, 1, fp);
}

int main(int argc, char** argv) {
	if (argc < 3) {
		show_help();
		return 0;
	}

	FILE* fp = fopen(argv[1],"rb");
	if (!fp) {
		perror("failed to open file");
		return -1;
	}

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0) {
		perror("failed to get socket descripter");
		goto err;
	}

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(FTRANS_PORT);
	addr_server.sin_addr.s_addr = inet_addr(argv[2]);

	if (connect(sockfd, (struct sockaddr*)&addr_server, sizeof(struct sockaddr_in)) < 0) {
		perror("failed to connect to server");
		goto err;
	}

	char buf[FTRANS_BUF_SIZE];
	size_t read_size = 0;
	while(read_size = load_data_from_file(fp, buf, FTRANS_BUF_SIZE)) {
		if (read_size <= 0)
			break;
	}

done:
	return 0;
err:
	if (fp)
		fclose(fp);
	if (sockfd > 0)
		close(sockfd);
	return -1;
}
