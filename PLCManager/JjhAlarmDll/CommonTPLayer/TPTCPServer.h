/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPTCPServer.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ��TCP������������

* ��ǰ�汾��1.1
* ��    �ߣ��ּ���
* ������ڣ�2008��4��28��

* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��4��28��

*
* ȡ���汾��1.0
* ԭ���ߡ���
* ������ڣ�
* �޶���¼��
*/
#ifndef TPTCPSERVER
#define TPTCPSERVER

#include "ITPListener.h"
#include "ITPObject.h"
#include "ThreadMutex.h"
#include "AX_Thread_Mutex.h"
#include "AX_Thread_Guard.h"
#include <deque>

class TPTCPServer : public ITPObject
{
public:
    TPTCPServer(ITPListener* tcpserverapp, int engineId = 0);
	TPTCPServer(int engineId, ITPListener* listener, CNewMutex* mutex);
    virtual ~TPTCPServer();

public: 
    virtual int CloseClient(int id);

    virtual int Close();
    
	virtual int Listen(char* ip, int port);
    
    virtual int Send(int id, char *pBuf, unsigned int iBufLen);

    virtual int Heartbeat();

	virtual Queue_List* GetSendQueue(int connId);

	virtual int SetMaxDataQueueLength(int maxDataQueueLength);

protected:
	int sendInside(int id, char* pBuf, unsigned int iBufLen);
	int closeInside();
	virtual int clearSendBuffer();	
	virtual int fillFds(int& maxfd, fd_set& readfds, fd_set& writefds);
	virtual int dealFDResult(int& fds,fd_set& readfds, fd_set& writefds,bool& fdsChange);


protected:
	int	_maxDataQueueLength;

protected:
	void addPendingCloseClient(int id);
	int dealPendingCloseClients();
	void pendingCloseClient(int id);
	std::deque<int> _pendingCloseClients;
	AX_Thread_Mutex _pendingCloseClientsMutex;
};

#endif

