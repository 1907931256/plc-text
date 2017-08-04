/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：TPTCPClient.h
* 文件标识：参见配置管理计划书
* 摘　　要：TCP客户端功能类
*
* 当前版本：1.1
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

