#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <sys/types.h>

#include <errno.h>


int main(int argc, char ** argv)
{
	int fd;
	struct sockaddr_in addr;
	struct timeval timeo = {5, 0};
	socklen_t len = sizeof(timeo);

	if (argc != 3) {
		printf("usage: %s ip port \n", argv[0]);
		return 1;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket error");
		return 1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) < 0) {
		perror("setsockopt error"); 
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
		if (errno == EINPROGRESS) {
			fprintf(stderr, "connect timeout\n");
			return 1;
		}
		perror("connect error");
		return 1;
	}
	printf("connect success\n");

	return 0;
}

