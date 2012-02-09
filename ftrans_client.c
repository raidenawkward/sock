#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

void show_help() {
	printf("usage: client [file] [target ip]\n");
}

int main(int argc, char** argv) {
	if (argc < 3) {
		show_help();
		return 0;
	}

	return 0;
}
