#include "yhsocket.h"
#include "yhprotocol.h"
#include <time.h>
#include <stdlib.h>

int
main(int argc, char ** argv)
{
	int fd;	 // server fd
	int cd;  // client fd
	int n;   // recvice len
	char buff[1024];
	if (argc != 2)
	{
		printf("usage: %s port\n", argv[0]);
		exit(1);
	}

	int port = atoi(argv[1]);
	fd = create_tcp_v4_server("0.0.0.0", port, 100);
	if (fd < 0) {
		perror("create server error");
		return 1;
	}

	srand(time(0));

	printf("create server success: service fd: %d, service port: %d\n", fd, port);

	time_t timet;
	struct tm tmt;

	char times[66] = {0};

	while (1)
	{
		cd = accept(fd, 0, 0);

		if (cd <= 0) continue;

		timet = time(0);
		tmt = *localtime(&timet);

		strftime(times, sizeof(times), "%F %T", &tmt);

		printf("%s get connect fd %d\n", times, cd);

		int i = 0;

		for(; ; i++)
		{
			int cmd  = rand() ;
			int n = __pack_send(cd, cmd, 0, -1);
			if (n <= 0)
				break;
			printf("%10d ... %d<cmd:%2d\n", i,  n, cmd);
			usleep(1);
		}
		close(cd);
	}
	return 0;
}

