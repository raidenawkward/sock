#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#include "ftrans.h"

void show_help() {
	printf("usage: client [file] [target ip]\n");
}

int main(int argc, char** argv) {
	if (argc < 3) {
		show_help();
		return 0;
	}

	int fp = open(argv[1],O_RDONLY);
	if (fp < 0) {
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

	char buf[FTRANS_SEND_BUF_SIZE];
	size_t read_size = 0;
	size_t total_size = 0;
	memset(buf, 0x00, sizeof(buf));

	while(1) {
		read_size = read(fp, buf, sizeof(buf));
		if (read_size == 0) {
			break;
		} else if (read_size < 0) {
			perror("error when reading data from file");
			goto err;
		}

		if (send(sockfd, buf, read_size, 0) != read_size) {
			perror("error detacted when uploading file");
			goto err;
		}

		total_size += read_size;
		memset(buf, 0x00, sizeof(buf));
	}

	printf("%d data sent\n", total_size);
done:
	close(fp);
	close(sockfd);

	return 0;
err:
	if (fp)
		close(fp);
	if (sockfd > 0)
		close(sockfd);
	return -1;
}
