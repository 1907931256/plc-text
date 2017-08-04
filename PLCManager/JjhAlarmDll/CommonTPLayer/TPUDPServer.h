/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：TPUDPServer.h
* 文件标识：参见配置管理计划书
* 摘　　要：UDP服务器实现类
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
    //关闭一个客户端
    virtual int CloseClient(int id);

    //关闭这个连接对象本身
    virtual int Close();
    
    //TPUDPServer要实现
    virtual int Listen(char* ip, int port);

    virtual int Send(int id,char * pBuf, unsigned int iBufLen);
    
    virtual int Heartbeat();
protected:
    int sendInside(int id, char *pBuf, unsigned int iBufLen);
	int closeInside();
};

#endif

