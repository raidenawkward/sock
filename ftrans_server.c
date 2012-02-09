#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

#include <pthread.h>
#include <semaphore.h>

#include "ftrans.h"

#define ETH_NAME "eth0"


#ifdef FTRANS_USE_SEM
sem_t sem_forks;
#endif

#ifdef FTRANS_USE_MUTEX
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

pthread_mutex_t listener_mutex;

void* forks_listener(void* arg) {
	while(1) {
		if (CURRENT_FORKS) {
			pid_t term_id = wait(NULL);

			pthread_mutex_lock(&listener_mutex);

			int index = get_fork_index(term_id);
			if (index < 0) {
				pthread_mutex_unlock(&listener_mutex);
				continue;
			}
			FORKS[index] = -1;
			--CURRENT_FORKS;

			pthread_mutex_unlock(&listener_mutex);
		}
	}
}
#endif //FTRANS_USE_MUTEX

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

int get_received_file_name(int index, char* name) {
	if (index < 0 || !name)
		return -1;

	return sprintf(name,"%d",index);
}

int receive_file_from_client_socket(int socketClient, const char* pathToSave) {
	if (socketClient < 0 || !pathToSave)
		return -1;

	int fp = open(pathToSave, O_WRONLY | O_CREAT, FTRANS_UPLOAD_MOD);
	if (fp < 0) {
		perror("failed to open file");
		return -1;
	}

	char buf[FTRANS_RECV_BUF_SIZE];
	memset(buf, 0x00, sizeof(buf));
	size_t sz_read = 0;
	size_t sz_total_read = 0;

	while(1) {
		sz_read = read(socketClient, buf, sizeof(buf));
		if (!sz_read) {
			break;
		} else if (sz_read < 0) {
			perror("error when reading data");
			goto err;
		}

		sz_total_read += sz_read;

		size_t write_bytes = write(fp, buf, sz_read); 
		if (write_bytes < sz_read || write_bytes < 0) {
			printf("error when write data to file\n");
			goto err;
		}

		memset(buf, 0x00, FTRANS_RECV_BUF_SIZE);
	}

	close(fp);

	return sz_total_read;

err:
	if (fp > 0)
		close(fp);
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

	printf("server launching..");

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

#ifdef FTRANS_USE_SEM
	if (sem_init(&sem_forks, 1, FTRANS_MAX_FORKS) < 0) {
		perror("failed to init sem for forks");
		goto err;
	}
#endif

#ifdef FTRANS_USE_MUTEX
	int i;
	for(i = 0; i < FTRANS_MAX_FORKS; ++i) {
		FORKS[i] = -1;
	}

	if (pthread_mutex_init(&listener_mutex, NULL) < 0) {
		perror("failed to create listener mutex");
		goto err;
	}

	pthread_t thread_listener;

	if (pthread_create(&thread_listener, NULL, forks_listener, NULL) != 0) {
		perror("failed to create listener thread");
		goto err;
	}
#endif

	printf(" done\n");
	printf("local ip: %s\n",inet_ntoa(addr_server.sin_addr));
	printf("server sockfd: %d\n", sockfd);

	int total_files = 0;

	while (1) {
		socklen_t socklen_client = 0;
		struct sockaddr_in addr_client;

		int sockfd_client = accept(sockfd, (struct sockaddr*)&addr_client, &socklen_client);
		if (sockfd_client < 0) {
			perror("error client connection detacted");
			continue;
		}

		printf("* quest received from %s\n",inet_ntoa(addr_client.sin_addr));

		char name[256];
		memset(name, 0x00, 256);
		if (get_received_file_name(++total_files, name) < 0) {
			continue;
		}
		char path[1024];
		memset(path,0x00, 1024);
		strcat(path, path_to_save);
		strcat(path, "/");
		strcat(path, name);

#ifdef FTRANS_USE_SEM
		sem_wait(&sem_forks);

		int pid = fork();
		if (pid < 0) {
			perror("failed to fork");
			sem_post(&sem_forks);
			continue;
		} else if (pid == 0) {
			size_t sz = receive_file_from_client_socket(sockfd_client, path);

			close(sockfd_client);
			sem_post(&sem_forks);

			if (sz >= 0) {
				printf("%d bytes data saved\n", sz);
				exit(0);
			} else {
				printf("some error detacted while file ftrans\n");
				exit(-1);
			}
		} else {

		}
#endif

#ifdef FTRANS_USE_MUTEX
		int new_pid_index = get_next_fork_index();
		if (new_pid_index < 0) {
			// list is full
			close(sockfd_client);
			continue;
		}

		FORKS[new_pid_index] = fork();

		if (FORKS[new_pid_index] < 0) {
			perror("failed to fork");
			FORKS[new_pid_index] = -1;
		} else if (FORKS[new_pid_index] == 0) {
			printf("saving file to %s\n",path);
			size_t sz = receive_file_from_client_socket(sockfd_client, path);

			close(sockfd_client);

			if (sz >= 0) {
				printf("%d bytes data saved\n", sz);
				exit(0);
			} else {
				printf("some error detacted while file ftrans\n");
				exit(-1);
			}
		} else {
			++CURRENT_FORKS;
		}
#endif
	} //while(1)

done:
	close(sockfd);
#ifdef FTRANS_USE_MUTEX
	pthread_mutex_destroy(&listener_mutex);
#endif
#ifdef FTRANS_USE_SEM
	sem_destroy(&sem_forks);
#endif
	return 0;

err:
	if (sockfd > 0)
		close(sockfd);

#ifdef FTRANS_USE_MUTEX
	pthread_mutex_destroy(&listener_mutex);
#endif

#ifdef FTRANS_USE_SEM
	sem_destroy(&sem_forks);
#endif

	return -1;
}
