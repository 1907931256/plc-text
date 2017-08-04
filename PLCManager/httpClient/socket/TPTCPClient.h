/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPTCPClient.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ��TCP�ͻ��˹�����
*
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
#ifndef _TPTCPClient_H_
#define _TPTCPClient_H_

#include "ITPObject.h"

class TPTCPClient : public ITPObject
{
public:
    TPTCPClient(ITPListener *tcpclientapp, int engineId = 0);
	//TPTCPClient(int engineId, ITPListener *tcpclientapp, base::internal::LockImpl* mutex);
    virtual ~TPTCPClient();

public:
    virtual int Close(void);

    virtual int Connect(const char* ip, int port);    

	virtual int Connect(const char* localIp, int localPort, const char* remoteIp, int remotePort);

    virtual int Send(int id, const char *pBuf, unsigned int iBufLen);

    virtual int Heartbeat(void);

	virtual int SetMaxDataQueueLength(int maxDataQueueLength);

	virtual int dealFDResult(int& fds,fd_set& readfds, fd_set& writefds,bool& fdsChange);

protected:
    int sendInside(int id, const char *pBuf, unsigned int iBufLen);
	int closeInside();

	int	_maxDataQueueLength;
};


#endif

