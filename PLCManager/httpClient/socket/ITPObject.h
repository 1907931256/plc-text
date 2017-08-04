/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�ITPObject.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ�������ӿڳ�����
*
* ��ǰ�汾��2.1
* ��    �ߣ��ּ���
* ������ڣ�2008��4��28��
* �޶���¼��
	1. �Ż�serverģʽ�µ����ݻ���ΪQueueJitt
	2. �޸�clientId�ļ��㷽��Ϊ���������ʽ

* ��ǰ�汾��2.0
* ��    �ߣ�������
* ������ڣ�2007��10��26��
* �޶���¼��
	1. ���ݻ�����������queue��
	2. ���Ӷ��иĳ���Hash_map
	3. ��RTP���ṩ��֧�֣���������Ԫ�ඨ�壬��sendInside��Ϊ���෽����
	4. �޸���_sequence�����Ա������

* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��4��28��

*
* ȡ���汾��1.0
* ԭ���ߡ���
* ������ڣ�
* �޶���¼��
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

	//���õײ��׽��ֻ�������С��type��ʾ�Ƿ��ͻ��������ǽ��ջ�����
	int SetSocketBufferSize(TPType type, int size);

	//��ȡ�ײ��׽��ֻ�������С��type��ʾ�Ƿ��ͻ��������ǽ��ջ�����
	int GetSocketBufferSize(TPType type);

	//���ô�������selectʱ�ĳ�ʱֵ���������ֵ��Ĭ����sec=0;usec=10;
	//�������Ϊ0���ʾΪ��ѭ״̬
	int SetSelectTimeout(long sec, long usec);

	//���ô������ջ�������С���������ɴ�����Լ�ά����Ӧ�ò��ָ����С
	int SetRecvTPBuffSize(int size);			

	//�������ý��ջ������������趨Ӧ�ó����Լ��Ľ��ջ�����������㽫����ֱ�ӽ����ڸ����С�
	//����Ļ�����Ҫ�ϲ����й����²㲻�������
	int SetTPRecvBuffer(char* buff, int size);

	//ʹ��nagle�㷨��1Ϊ�򿪣�0Ϊ�ر�
	int SetNodelayFlag(int flag);

	//���û��峤�Ȼص��ķ�ֵ�����Ͷ��г��ȵ���threshold�ı���ʱ�ص�֪ͨ�ϲ㣬threadhold=0����֪ͨ
	//�ص��ĳ����ǵ�ʱ���Ͷ��е���ʵ���ȣ�����һ����׼ȷ��threshold�ı���
	int SetSendQueueThreshold(int threshold);

	//��ȡ���ͻ������
	//����ֵ���ڲ����ͻ�����е�ָ�룬ʹ�ô˽ӿ�Ӧ����
	//Ӧ��heartbeat()��senddata()�ȿ��ܸı䷢�Ͷ��еĽӿڻ���
	virtual Queue_List* GetSendQueue(int connId);


	// һ���Լ�ʱ�����췽��
	//delay		��ʼ�ӳ�
	//context   �����ģ�ontimerʱ��ص���ֵ
	//return	>=0ʱ�������������ڹر�ʱ�ӣ�<0ʧ��
	long SchedureTimer(int delay, int context);

	//ѭ���Լ�ʱ�����췽��
	//interval	ʱ�����ڣ���λ������
	//delay		��ʼ�ӳ�
	//context   �����ģ�ontimerʱ��ص���ֵ
	//return	>=0ʱ�������������ڹر�ʱ�ӣ�<0ʧ��
	long SchedureRepeatTimer(int interval, int delay, int context);

	//ȡ����ʱ�� ��һ����ʱ�ӻص�һ�κ��Զ�ʧЧ��ѭ����ʱ��Ҳ��ͨ��timeout����ֵ��ȡ����
	//timerId		��ʱ������
	//return		0�ɹ���<0ʧ��
	int CancelTimer(long timerId);

	virtual int clearSendBuffer();

public:	

	//������ʼ���������������
	static void Startup(void);
	static void Cleanup(void);

	unsigned int	_ip;		// IP,PORT ���������ֽ���
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


