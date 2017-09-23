/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPTypedef.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ�������Ԥ����
*
* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��4��28��

*
* ȡ���汾��1.0
* ԭ���ߡ���
* ������ڣ�
* �޶���¼��
*/
#ifndef _TPETYPEDEF_H_
#define _TPETYPEDEF_H_

#if defined(WIN32) || defined(WINCE)
//#include "winsock2i.h"
#include <hash_map>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/tcp.h>		/* TCP_NODELAY */
#include <pthread.h>
#include <arpa/inet.h>	/* inet(3) functions */

#include <ext/hash_map>
using namespace __gnu_cxx;

#include <unistd.h>

#define closesocket(S) close(S)
#endif

#include <vector>
#include <stdio.h>
#include <stdlib.h>

#if !defined(WINCE)
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>		/* for nonblocking */
#endif

#include <string.h>
#include "platform.h"

#define BUF_SIZE 64*1024

#ifndef WIN32
#define INVALID_SOCKET		-1
#endif

#define MAX_ACCEPT_DLGS  20

#define INVALID_LOCAL_ADDRESS				-10
#define NOT_DEFINE_LOCAL_ADDRESS			-11

#define TP_ERROR_BASE						-1
#define TP_NORMAL_RET						(TP_ERROR_BASE + 1)
#define TP_NOTHING_IS_DONE_RET				(TP_NORMAL_RET + 1)
#define TP_ERROR_UNSUPPORT					(TP_ERROR_BASE - 100)
#define TP_ERROR_BAD_CONNECTION				(TP_ERROR_BASE - 101)
#define TP_ERROR_SET_NOBLOCKING_FAILED		(TP_ERROR_BASE - 102)


#ifndef SOCKET_ERROR
#define SOCKET_ERROR		-1
#endif

#define TP_SELF	-11

typedef struct
{
    unsigned int	ip;
	unsigned short	port;
	unsigned short	online;		//0:using 1:unusing
	int				socket;
    unsigned int	id;
    unsigned int	timemark;
}client_list;

typedef enum
{
	TP_SEND = 1,
	TP_RECEIVE
} TPType;

typedef struct
{
	uint64 delay;
	uint64 interval;
	int repeat;
	int context;

	uint64 timeBegin;
	uint64 lastTimeout;
}tp_timer;

#endif
