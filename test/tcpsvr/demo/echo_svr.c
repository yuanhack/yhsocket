#include "../../yhsocket.h"
#include <time.h>

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

	printf("create server success: service fd: %d, service port: %d\n", fd, port);

	while (1)
	{
		cd = accept(fd, 0, 0);

		if (cd <= 0)
			continue;

		char times[66] = {0};
		time_t timet;
		struct tm tmt;
		timet = time(0);
		tmt = *localtime(&timet);
		strftime(times, sizeof(times), "%F %T", &tmt);
		printf("%s get connect fd %d\n", times, cd);

		while ((n = read(cd, buff, sizeof(buff))) > 0)
		{
			write(1, buff,  n);
			write(1, "\n", 1);
			break;
		}
		close(cd);
	}
	return 0;
}
