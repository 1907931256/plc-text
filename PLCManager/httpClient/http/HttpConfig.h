
//http 操作配置功能


#ifndef _HTTP_CONFIG_H_
#define _HTTP_CONFIG_H_

#include "TPTCPClient.h"
#include "ITPListener.h"
#include "DH_HTTP.h"
#include "string_util.h"
#include "AX_Mutex.h"
#include "AX_IAddRefAble.h"
#include "AX_Thread.h"
#include <map>
#include <vector>

typedef struct
{
	std::string user;
	std::string pass;
	std::string query;
	std::string ip;
	int reSendTime;
}httpSendInfo;
typedef std::map<std::string,httpSendInfo> httpSendInfoMap; //使用 query 作为key
//std::vector<httpSendInfo>sendItemVec;
	

class CCuHttpConfigClient : public ITPListener
{
public:
	CCuHttpConfigClient();
	virtual ~CCuHttpConfigClient();

	bool SetIP(const char* ip, const int port);
	bool SetQueryStr(const char* query_str);
	bool SetUser(const char* name, const char* passwd);
	bool SetChn(const int chn_start, const int chn_num);

	bool SetJsonBody(const char* body);

	bool Start(std::string& output);
public: //!ITPListener
	virtual int onData(int engineId, int connId, const char* data, int len);
	virtual int onClose(int engineId, int connId);
	virtual int onConnect(int engineId, int connId, const char* ip, int port);
	virtual int onSendDataAck(int engineId, int id, int nSeq, int sendLen);
	virtual int onSendStatus(int engineId, int connId, int statusType, int param);
	virtual int onTimeout(int id, int context);
protected:
	TPTCPClient* _connection;
	std::string m_ip;
	int m_port;
	std::string m_username;
	std::string m_passwd;
	std::string m_query_str;
	int	m_chn_start;
	int m_chn_num;
	std::string m_json_body;

	bool m_bResponse;
	bool m_bDigist;

	std::string	m_recv_buf;

	HTTPResponse*		m_httpReponse;

public:
	static bool StartCGI(std::string& url);
	static void reSendStart();
	static void* workThreadStartSend(void *lpParam);	
	static void RunReSend();

	//sendItemVec mSendItem;
	//static httpSendInfoMap mSendItem;
	//static HANDLE m_ReSendThread;
	//static AX_Mutex m_lockSendItem;	
};


#endif