#pragma once

#include <Winsock2.h>
#include "TPTCPClient.h"
#include "ITPListener.h"
class GetJjhAlarmInfo;
class GetJjhAlarmClient : public ITPListener
{  
public:  
	GetJjhAlarmClient(GetJjhAlarmInfo *pParent);
	~GetJjhAlarmClient();
public:
	bool SetIP(const char* ip, const int port);
	void ReSetRespance(BOOL bState = FALSE){m_bResponse = bState;}

	bool Connect(/*std::string& output*/);
	void SendRequest(char *pRequest, int iLength);
public: //!ITPListener
	virtual int onData(int engineId, int connId, const char* data, int len);
	virtual int onClose(int engineId, int connId);
	virtual int onConnect(int engineId, int connId, const char* ip, int port);
	virtual int onSendDataAck(int engineId, int id, int nSeq, int sendLen);
	virtual int onSendStatus(int engineId, int connId, int statusType, int param);
	virtual int onTimeout(int id, int context);

protected:
	bool m_bConnect;
	TPTCPClient* _connection;
	std::string m_ip;
	int m_port;
	std::string m_query_str;
	bool m_bResponse;
	std::string	m_recv_buf;
	GetJjhAlarmInfo *m_pParent;

private:

};  
