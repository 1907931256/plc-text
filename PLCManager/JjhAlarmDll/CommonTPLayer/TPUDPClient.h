/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：TPUDPClient.h
* 文件标识：参见配置管理计划书
* 摘　　要：UDP客户端功能类
*
* 当前版本：1.2
* 作    者：林坚彦
* 完成日期：2008年4月28日

* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年4月28日

*
* 取代版本：1.0
* 原作者　：
* 完成日期：
* 修订记录：
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

