/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：ITPObject.h
* 文件标识：参见配置管理计划书
* 摘　　要：传输层接口抽象类
*
* 当前版本：2.1
* 作    者：林坚彦
* 完成日期：2008年4月28日
* 修订记录：
	1. 优化server模式下的数据缓冲为QueueJitt
	2. 修改clientId的计算方法为句柄递增方式

* 当前版本：2.0
* 作    者：李明江
* 完成日期：2007年10月26日
* 修订记录：
	1. 数据缓冲区换成了queue。
	2. 连接队列改成了Hash_map
	3. 对RTP库提供了支持，增加了友元类定义，将sendInside改为基类方法。
	4. 修改了_sequence这个成员的名字

* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年4月28日

*
* 取代版本：1.0
* 原作者　：
* 完成日期：
* 修订记录：
*/
#ifndef _ITPOBJECT_H_
#define _ITPOBJECT_H_

#include "TPTypedef.h"
#include "ITPListener.h"
#include <assert.h>
#include "DataRow.h"

#include <queue>
#include <map>

#ifdef  WIN32
#pragma   warning(disable   :   4996) 
#pragma   warning(disable   :   4786) 
#endif

typedef std::queue<DataRow*> Queue_List;
#ifdef WIN32
typedef stdext::hash_map<int, Queue_List*> Queue_Jitt;
#else
typedef hash_map<int, Queue_List*> Queue_Jitt;
#endif

typedef std::map<int, int> Int_Map;

#ifdef WIN32
typedef stdext::hash_map<unsigned int, client_list*> CONN_MAP;
#else
typedef hash_map<unsigned int, client_list*> CONN_MAP;
#endif

typedef std::map<int, tp_timer*> TIMER_MAP;

class CRTPSession;
class CRTPServerBase;
class CLinuxRTPServer;

class ITPObject : public RefCounted<ITPObject>
{
public:   
	ITPObject(ITPListener* instance, int engineId = 0);

    virtual ~ITPObject();
	
	void SetListener(ITPListener* listener);

	
	virtual int Connect(const char* ip, int port);
	virtual int Connect(const char* localIp, int localPort, const char* remoteIp, int remotePort);
    
	virtual int Listen(char* ip, int port);

	virtual int Send(int id, const char *pBuf, unsigned int iBufLen) = 0;
    
	virtual int Close(void);
	virtual int CloseClient(int id);

    virtual int Heartbeat() = 0;

	client_list* GetConnection(int id);
	int SetConnection(client_list* conn);

	friend class CRTPSession;
	friend class CRTPServerBase;
	friend class CLinuxRTPServer;
	//friend class CRTPAppSession
public:

	//设置底层套接字缓冲区大小，type表示是发送缓冲区还是接收缓冲区
	int SetSocketBufferSize(TPType type, int size);

	//获取底层套接字缓冲区大小，type表示是发送缓冲区还是接收缓冲区
	int GetSocketBufferSize(TPType type);

	//设置传输层调用select时的超时值。如果不赋值则默认设sec=0;usec=10;
	//如果设置为0则表示为轮循状态
	int SetSelectTimeout(long sec, long usec);

	//设置传输层接收缓冲区大小，缓冲区由传输层自己维护，应用层仅指定大小
	int SetRecvTPBuffSize(int size);			

	//用于设置接收缓冲区，可以设定应用程序自己的接收缓冲区，传输层将数据直接接收在该区中。
	//传入的缓冲区要上层自行管理，下层不负责管理。
	int SetTPRecvBuffer(char* buff, int size);

	//使能nagle算法。1为打开，0为关闭
	int SetNodelayFlag(int flag);

	//设置缓冲长度回调的阀值，发送队列长度到达threshold的倍数时回调通知上层，threadhold=0禁用通知
	//回调的长度是当时发送队列的真实长度，并不一定是准确的threshold的倍数
	int SetSendQueueThreshold(int threshold);

	//获取发送缓冲队列
	//返回值是内部发送缓冲队列的指针，使用此接口应慎重
	//应与heartbeat()、senddata()等可能改变发送队列的接口互斥
	virtual Queue_List* GetSendQueue(int connId);


	// 一次性计时器构造方法
	//delay		开始延迟
	//context   上下文，ontimer时会回调该值
	//return	>=0时钟索引，可用于关闭时钟，<0失败
	long SchedureTimer(int delay, int context);

	//循环性计时器构造方法
	//interval	时钟周期，单位：毫秒
	//delay		开始延迟
	//context   上下文，ontimer时会回调该值
	//return	>=0时钟索引，可用于关闭时钟，<0失败
	long SchedureRepeatTimer(int interval, int delay, int context);

	//取消计时器 （一次性时钟回调一次后自动失效，循环性时钟也可通过timeout返回值来取消）
	//timerId		计时器索引
	//return		0成功，<0失败
	int CancelTimer(long timerId);

	virtual int clearSendBuffer();

public:	

	//传输层初始化，在主程序入口
	static void Startup(void);
	static void Cleanup(void);

	unsigned int	_ip;		// IP,PORT 都是网络字节序
	int				_socket;	
	unsigned short	_port;

public:
	static long GetNewClientId();

    static QAtomicInt _newClientId;
protected:
	unsigned int	_localIp;
	unsigned short	_localPort;

	ITPListener*	_listener;
	QMutex*		_mutex;
	CONN_MAP		_clients;
	Queue_List		_queue;//for tcp_client&udp_both
	Queue_Jitt		_queueJitt;	//for tcp_server
	int				_engineId;
	unsigned short	_sequenceNo;
	int				_nodelay;
	
	int				_recvBuffSize;
	int				_sendBufferSize;	
	int				_tpRecvBuffSize;
	int				_localBuffer;

	struct timeval	_timeout;
	char*			_buffer;

	CDataRowPool	_dataRowPool;

	int				_sendQueueThreshold;
	int				_lastAnouncedThreshold;

	TIMER_MAP		_timerMap;

    QAtomicInt _newTimerId;
	long getTimerId(void);

	typedef struct  
	{
		int id;
		int context;
		int repeat;
	}timeout_cb_param;
	int checkTimer();

	DataRow* createDataRow();
	int getSequence(void);
	virtual int sendInside(int id, const char *pBuf, unsigned int iBufLen) = 0;	
	virtual int fillFds(int& maxfd, fd_set& readfds, fd_set& writefds);
	virtual int dealFDResult(int& fds,fd_set& readfds, fd_set& writefds,bool& fdsChange);

};

//long getTime();

#endif


