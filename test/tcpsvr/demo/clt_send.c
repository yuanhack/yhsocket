#include <errno.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "yhsocket.h"
#include "yhprotocol.h"

int fd;
static int count = 0;

void sig_quit(int no)
{
	char buf[1024*1024];
	int n = recv(fd, buf, sizeof(buf), 0);
	if (n < 0) {
		if(errno == EINTR) {
			printf("EINTR %d recv %s\n", errno, strerror(errno));	
		}
		else if (errno == EAGAIN) {
			printf("EAGAIN %d recv: %s\n", errno, strerror(errno));	
		} else 
			printf("recv() error: %d: %s\n", errno, strerror(errno));
	} else if (n == 0) {
		printf("recv close\n");
		exit(0);
	} else {
		printf("recv len=%d\n", n);
	}
	n = nsend(fd, buf, sizeof(buf), 0);
	if (n <= 0) {
		if (n < 0) {
			if(errno == EINTR) {
				printf("EINTR %d nsend: %s\n", errno, strerror(errno));	
				return;
			}
			else if (errno == EAGAIN) {
				printf("EAGAIN %d nsend: %s\n", errno, strerror(errno));
				return;
			} else 
				printf("nsend() error: %d: %s\n", errno, strerror(errno));
		}
		else if (n == 0)
			printf("connection refused\n");
		exit(0);
	}
	printf("nsend count: %d, recv len=%fM\n", ++count, n/1024.0/1024);
}


int
main(int argc, char ** argv)
{

	if (argc != 3) {
		printf("usage: %s ip port\n", argv[0]);
		return 1;
	}

	signal(SIGQUIT, sig_quit);

	fd = create_tcp_v4_connect(argv[1], atoi(argv[2]));
	if (fd < 0) {
		if (fd == -2)
			printf("%s: ip port error\n", argv[0]);
		else
			printf("%s: connect to %s:%s error: %s\n", argv[0], argv[1], argv[2], strerror(errno));
		return 1;
	}
	
	struct timeval send_out = {3, 0};	
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &send_out, sizeof(send_out));	
	struct timeval recv_out = {0, 10};	
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &send_out, sizeof(send_out));	
	
	printf("%s: connect to %s:%s\n", argv[0], argv[1], argv[2]);


	while (1) {
		char buf[1024*1024];
		int n = nsend(fd, buf, sizeof(buf), 0);
		if (n <= 0) {
			if (n < 0) {
				if(errno == EINTR) {
					printf("EINTR %d recv: %s\n", errno, strerror(errno));
					continue;
				}
				else if (errno == EAGAIN) {
					printf("EAGAIN %d recv: %s\n", errno, strerror(errno));	
					continue;
				} else {
					printf("nsend() error: %d: %s\n", errno, strerror(errno));
					break;
				}
			} else if (n == 0)
				printf("connection refused\n");
			exit(0);
		}
		printf("nsend count: %d, recv len=%fM\n", ++count, n/1024.0/1024);	
	}

	close(fd);

	return 0;
}

