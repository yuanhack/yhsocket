#ifndef __RCYH_YHPROTOCOL_H__
#define __RCYH_YHPROTOCOL_H__


#include "yhsocket.h"

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push)
#pragma pack(4)

#define __MAGICYH        0x0123ABCD

typedef unsigned int  uint_t;

typedef struct header
{
	uint_t magic;
	int    cmd;
	int    len;
	char   data[];
} header_t;

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

#pragma pack(pop)

void __pack_make(header_t *pbody, int cmd, void *ptr, int len);

// result
//  -1 : socket local error
//  -2 : memory missing, memory allocation failed
//  -3 : data is null or len < 0
int  __pack_send(int s, int cmd, void *data, int len);

// result 
//   0 : socket remote closed
//  -1 : socket local error
//  -2 : memory missing, memory allocation failed
//  -3 : checkin MAGICYH and header_t.len error
int  __pack_recv(int s, header_t **pack);


#ifdef __cplusplus
}
#endif
#endif /* __RCYH_YHPROTOCOL_H__ */
