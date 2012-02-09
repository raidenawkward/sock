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
	printf("usage: client file targetip [target name]\n");
}

const char* get_file_name_from_path(const char* path) {
	if (!path)
		return NULL;
	char* ret = strrchr(path, '/'); 
	if (!ret)
		return path;
}

size_t get_file_size(const char* path) {
	if (!path)
		return -1;

	struct stat s;
	if (stat(path, &s) < 0)
		return -1;

	return s.st_size;
}

char* itoa(int i) {
	int buf_size = sizeof(char) * sizeof(int);
	char* ret = (char*)malloc(buf_size);
	memset(ret,'\0', buf_size);

	if (i < 0) {
		ret[0] = '-';
		i = i * (-1);
	}

	int mod_seed = 10;
	int i_test = i;
	int i_length = i < 0? 2 : 1;
	while(1) {
		i_test = i_test / mod_seed;
		if (i_test > 0)
			++i_length;
		else
			break;
	}

	i_test = i;
	int index = i_length - 1;
	while(1) {
		int mod = i_test % mod_seed;
		ret[index--] = 48 + mod;

		i_test = i_test / mod_seed;

		if (i_test <= 0 || index < 0)
			break;
	}

	printf("size: %s\n", ret);
	return ret;
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

	// header = size + name
	char name_to_save[FTRANS_TRANS_HEADER_FILENAME];
	memset(name_to_save, 0x00, sizeof(name_to_save));
	if (argc > 3) {
		strcpy(name_to_save, argv[3]);
	} else {
		strcpy(name_to_save, get_file_name_from_path(argv[1]));
	}

	// first send header
	//
	char header[FTRANS_TRANS_HEADER_SIZE];
	memset(header, 0x00, sizeof(header));

	char header_size[32];
	size_t file_size = get_file_size(argv[1]);
	if (file_size < 0) {
		printf("invalid file size: %d\n", file_size);
		goto err;
	}
	strncpy(header_size, itoa(file_size), sizeof(header_size));

	memcpy(header, header_size, sizeof(header_size));
	memcpy(header + sizeof(header_size), name_to_save, sizeof(name_to_save));

	if(send(sockfd, header, sizeof(header), 0) <= 0) {
		perror("error when reading header data");
		goto err;
	}

	// second send file content
	//
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
