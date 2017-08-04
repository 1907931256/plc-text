/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�TPUDPServer.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ��UDP������ʵ����
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
#ifndef TPUDPSERVER
#define TPUDPSERVER

#include "ITPListener.h"
#include "ITPObject.h"
#include "ThreadMutex.h"

class CRTPSession;

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
class TPUDPServer : public ITPObject
{
public:
    TPUDPServer(ITPListener* callback, int engineId = 0);
    TPUDPServer(int engineId, ITPListener* callback, CNewMutex* mutex);
	virtual ~TPUDPServer();

    //<Methods>
public:        
    //�ر�һ���ͻ���
    virtual int CloseClient(int id);

    //�ر�������Ӷ�����
    virtual int Close();
    
    //TPUDPServerҪʵ��
    virtual int Listen(char* ip, int port);

    virtual int Send(int id,char * pBuf, unsigned int iBufLen);
    
    virtual int Heartbeat();
protected:
    int sendInside(int id, char *pBuf, unsigned int iBufLen);
	int closeInside();
};

#endif

