
//http ²Ù×÷ÅäÖÃ¹¦ÄÜ

#ifndef _HTTP_CONFIG_H_
#define _HTTP_CONFIG_H_
#include "HTTP/DH_HTTP.h"
#include "CommonTPLayer/TPTCPClient.h"
#include "CommonTPLayer/ITPListener.h"

//#include "json/json.h"
#define HTTPRESPONCE_TIME 1500

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
	void ReSetRespance(BOOL bState = FALSE){m_bResponse = bState;}

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

	bool	m_bResponse;

	std::string	m_recv_buf;

	HTTPResponse*		m_httpReponse;
};

//int GetJSValue_CString(Json::Value& table, const char *name,char *pDst,int DstLen);
//int GetJSValue_Int(Json::Value& table,const char *name,int &iValue);

#endif