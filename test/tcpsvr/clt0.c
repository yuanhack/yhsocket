#include <errno.h>
#include <string.h>

#include "yhsocket.h"
#include "yhprotocol.h"

int
main(int argc, char ** argv)
{
	int fd;
	char *p;

	if (argc != 3) {
		printf("usage: %s ip port\n", argv[0]);
		return 1;
	}

	p = argv[3];

	fd = create_tcp_v4_connect(argv[1], atoi(argv[2]));

	if (fd < 0) {
		if (fd == -2)
			printf("%s: ip port error\n", argv[0]);
		else
			printf("%s: connect to %s:%s error: %s\n", argv[0], argv[1], argv[2], strerror(errno));
		return 1;
	}
//	printf("%s: connect to %s:%s\n", argv[0], argv[1], argv[2]);

	int n;
	int *pi;
	header_t *pack;
	while ((n = __pack_recv(fd, &pack)) > 0)
	{
		printf("%d>cmd:%2d\n", n, pack->cmd, *pi);
		free(pack);
	}
	printf("-----%d\n", n);

	close(fd);

	return 0;
}
