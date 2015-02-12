#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/types.h>

#include <errno.h>


#define __MAGICYH        0x0123ABCD
#define __DATALEN        (1024*4)

// 请求反向连接
//   S  -> C 
//   data
//   "a=ResID:301001-0016\r\n"
//   "a=WndIndex:1\r\n"
#define CMD_CONNECT            0x0000  
// 接受反向连接
#define CMD_ACCEPT             0x1001  // C  -> S response connect
// 拒绝反向连接
#define CMD_REJECT             0x1002  // C  -> S response connect 

// 断开连接
#define CMD_DISCONNECT         0x0001  // C <-> S disconnect

// 通知客户端发送完毕某段数据
#define CMD_SOURCE_FIN         0x0002  // S  -> C

// 数据流包体
#define CMD_STREAM_DATA        0x0003  // S  -> C

// 播放控制: 速度
// C  -> S 
//   datas 
//   "Scale: x\r\n"x :
//   Normal 1
//   Quick  2     4    8 
//   Slow   0.125 0.25 0.5 
#define CMD_PLAY_SCALE         0x0011  

// 播放控制: 时间刻度
// NTP时间, 从1979年1月1日0时0分 到某个时间的秒数
// C-S 
//   data
//   "Range: ntp=n-\r\n"
#define CMD_PLAY_RANGE         0x0012

// 播放控制: 暂停
// C-S
#define CMD_PLAY_PAUSE         0x0004  // C  -> S
// 播放控制: 暂停恢复/继续
#define CMD_PLAY_CONTINUE      0x0005  // C  -> S


typedef unsigned int  uint_t;

typedef struct header
{
	uint_t magic;             // __MAGICYH
	int    cmd;               // CMD_xxxx
	int    len;               // data's len
	char   data[];
} header_t;

#define HEADERSIZE sizeof(header_t)


// create TCP/IP version 4 socket server
//-
// type: SOCK_STREAM, SOCK_DGRAM ...
// addr: struct sockaddr data
// alen: sizeof(struct sockaddr)
// qlen: The maximum length of the queue
//-
// [ret]: -1: create socket server failed, set errno
// [ret]: > 0: create successful, value is server socket
int create_socket_v4_server_for_addr
	(int type, const struct sockaddr *addr, socklen_t alen, int qlen)
{
	int is;
	int reuse = 1;

	if ((is = socket(addr->sa_family, type, 0)) < 0)
		goto errquit;
		
	/* 
	 * set SO_REUSEADDR for server socket 
	 */
	if (setsockopt(is, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
		goto errquit;

	if (bind(is, addr, alen) < 0) 
		goto errquit;	

	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) 
	{
		if (listen(is, qlen) < 0)
			goto errquit;
	}
	return is;
errquit:
	close(is);
	return -1;
}

// create TCP/IP version v4 socket server
//-
// type: SOCK_STREAM, SOCK_DGRAM ...
// ip  : x.x.x.x format ip address 
// port: service port
// listenlen: the length of listen list
//- 
// [ret]: -1: create socket server failed, set errno
// [ret]: > 0: create successful, value is server socket
int create_socket_v4_server(int type, char* ip, int port, int listenlen)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	return create_socket_v4_server_for_addr
		(type, (struct sockaddr*)&addr, sizeof(struct sockaddr), listenlen);
}

// Create socket version 4 connection
//-
// family: AF_INET ...
// type  : SOCK_STREAM, SOCK_DGRAM ...
// p_addr: x.x.x.x ip address
// port  : number port
//-
// [ret] : -2: ip or port error, no set errno value
// [ret] : -1: socket error, is set errno value
// [ret] : > 0: successful connection socket
int create_socket_v4_connect(int family, int type, const char* p_addr, int port)
{
	struct sockaddr_in saddr;
	int err, is;
	saddr.sin_port = htons(port);
	saddr.sin_family = family;
	err = inet_pton(family, p_addr, &saddr.sin_addr.s_addr);

	if (err == 0 || saddr.sin_port<=0)
		return -2;
	else if (err < 0)
		return -1;
		
	if ((is = socket(family, type, 0)) < 0)
		return -1;

	// connect timeout times
	struct timeval con_timeo = {30, 0};	
	setsockopt(is, SOL_SOCKET, SO_SNDTIMEO, &con_timeo, sizeof(con_timeo));

	if (connect(is, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
	{
		close(is);
		return -1;
	}
		
	return is;
}

// Send data to socket
//-
// fd  : destination socke
// buff: buffer
// len : buffer length
// flag: socket flags
// -
// [ret]: -1: error
//        errno set to indicate the error.
// [ret]: < len && > 0: error
//        Send incomplete length, Length is not len
// [ret]: > 0: Send completed successful.
int nsend(int fd, const void *buff, size_t len, int flag)
{
	size_t 	n;
	ssize_t nsend;
	n = len;
	while (n > 0)
	{
		if ((nsend = send(fd, buff, n, flag)) < 0)
		{
			if (errno == EINTR)
				continue;
			if (n == len)
				return(-1);	
			else
				break;
		}
		else if (nsend == 0)
		{	
			break;
		}
		n -= nsend; 	
		buff += nsend;
	}
	return len - n;
}



// Recvice data from socket
//-
// fd  : source socket
// buff: buffer
// len : buffer length
// flag: socket flags
// -
// [ret]: -1: error
//        errno set to indicate the error.
// [ret]: < len && > 0: error
//        Received incomplete length, Length is not len
// [ret]: > 0: Recv completed successful.
int nrecv(int fd, void *buff, size_t len, int flag)
{
	size_t n;
	ssize_t nrecv;

	n = len;
	while (n > 0) 
	{
		if ((nrecv = recv(fd, buff, n, flag)) < 0) 
		{
			if (errno == EINTR)
				continue;
			else if (len == n)
				return -1;
			else
				break;
		}
		else if (nrecv == 0)
			break;
		n -= nrecv;
		buff += nrecv;
	}
	return len - n;
}

void __pack_make(header_t *pbody, int cmd, void *ptr, int len)
{
	pbody->magic  = __MAGICYH;
	pbody->cmd    = cmd;
	pbody->len    = len;
	if (len > 0 && ptr != 0)
	{
		pbody->len = len;
		memcpy(pbody->data, ptr, len);
	}
	else
		pbody->len = 0;
	return;
}


#ifndef offsetof
#define offsetof(type, member) (size_t)&(((type *)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member)  \
	({\ const typeof(((type *)0)->member) * __mptr = (ptr);\
	 (type *)((char *)__mptr - offsetof(type, member)); \ })
#endif

#ifndef headersize
#define headersize  offsetof(header_t, data)
#endif

// result
//  -1 : socket local error
//  -2 : memory missing, memory allocation failed
//  -3 : data is null or len < 0
int  __pack_send(int s, int cmd, void *data, int len)
{
	if (len < 0 || (len > 0 && !data))
		return -3;
	header_t *pack = (header_t*)malloc(headersize + len);
	if (pack == 0)
		return -2;
	__pack_make(pack, cmd, data, len);
	int n  = nsend(s, pack, headersize + len, 0);
	free(pack);
	return n; // if send error return -1;
}

// result 
//   0 : socket remote closed
//  -1 : socket local error
//  -2 : memory missing, memory allocation failed
//  -3 : checkin MAGICYH and header_t.len error
int  __pack_recv(int s, header_t **pack)
{

	int n, size = 0;
	header_t head;
	
	size = nrecv(s, &head, headersize, 0);
	if (headersize != size)
		return size;  // socket result 0 or -1
	else if (head.magic != __MAGICYH || head.len < 0)
		return -3;    // check MAGIC error

	*pack = (header_t*)malloc(headersize + head.len);
	if (!*pack)
		return -2;    // error memory messing

	if (head.len > 0)
	{
		n = nrecv(s, ((char*)*pack) + headersize, head.len, 0);
		if (head.len != n)
		{
			free(*pack);
			return n;  // socket result 0 or -1
		}
		size += n;
	}

	**pack = head;

	return size;
}




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

//int  __pack_send(int s, int cmd, void *data, int len)
	char buff[1024] = {0};
	snprintf(buff, sizeof(buff), 
			"a=ResID:301001-0016\r\n"
			"a=WndIndex:1\r\n");
	int n;
	n = __pack_send(fd, CMD_CONNECT, buff, strlen(buff)+1);
	printf("send %d\n", n);
	
	header_t *pack;
	__pack_recv(fd, &pack);
	printf("cmd: %x\n", pack->cmd);
	printf("len: %d\n", pack->len);



	while (1)
		sleep(100);

	return 0;
}

