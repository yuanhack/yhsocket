#include "yhprotocol.h"


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


