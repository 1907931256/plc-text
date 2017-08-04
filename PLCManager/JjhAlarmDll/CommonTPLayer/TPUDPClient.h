/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPUDPClient.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ��UDP�ͻ��˹�����
*
* ��ǰ�汾��1.2
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
#ifndef TPUDPCLIENT
#define TPUDPCLIENT

#include "ITPObject.h"
#include "TPTypedef.h"
#include "ThreadMutex.h"

class CRTPSession;

class TPUDPClient : public ITPObject
{
public:
    TPUDPClient(ITPListener *callback, int engineId = 0);
	TPUDPClient(int engineId, ITPListener* callback, CNewMutex* mutex);
    virtual ~TPUDPClient();

	friend class CRTPSession;
public:
    virtual int Close();

    virtual int Connect(const char* ip, int port);

	virtual int Connect(const char* localIp, int localPort, const char* remoteIp, int remotePort);

    virtual int Send(int id, char *pBuf, unsigned int iBufLen);

    virtual int Heartbeat();
protected:
    int sendInside(int id, char *pBuf, unsigned int iBufLen);
	int closeInside();
};

#endif

