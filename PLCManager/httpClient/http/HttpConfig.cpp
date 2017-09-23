
#include "HttpConfig.h"
#include "string_util.h"

#include "our_md5.h"
#include "base64/base64.h"

httpSendInfoMap mSendItem;
HANDLE m_ReSendThread;
AX_Mutex m_lockSendItem;
int ez_base64encode(char *encoded, const char *string, int len)
{
	std::string tmp;
	base::Base64Encode(string,&tmp);

	strcpy(encoded,tmp.c_str());
	return 0;
}
char const* computeDigestResponse(char const* cmd, char const* realm, char const* nonce, char const* url, char const* username, char const* password, bool fPasswordIsMD5= false)
{
	// The "response" field is computed as:
	//    md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))
	// or, if "fPasswordIsMD5" is True:
	//    md5(<password>:<nonce>:md5(<cmd>:<url>))
	char ha1Buf[33];
	if (fPasswordIsMD5) {
		strncpy(ha1Buf, password, 32);
		ha1Buf[32] = '\0'; // just in case
	} else {
		unsigned const ha1DataLen = strlen(username) + 1
			+ strlen(realm) + 1 + strlen(password);
		unsigned char* ha1Data = new unsigned char[ha1DataLen+1];
		sprintf((char*)ha1Data, "%s:%s:%s", username, realm, password);
		our_MD5Data(ha1Data, ha1DataLen, ha1Buf);
		delete[] ha1Data;
	}

	unsigned const ha2DataLen = strlen(cmd) + 1 + strlen(url);
	unsigned char* ha2Data = new unsigned char[ha2DataLen+1];
	sprintf((char*)ha2Data, "%s:%s", cmd, url);
	char ha2Buf[33];
	our_MD5Data(ha2Data, ha2DataLen, ha2Buf);
	delete[] ha2Data;

	unsigned const digestDataLen
		= 32 + 1 + strlen(nonce) + 1 + 32;
	unsigned char* digestData = new unsigned char[digestDataLen+1];
	sprintf((char*)digestData, "%s:%s:%s",
		ha1Buf, nonce, ha2Buf);
	char const* result = our_MD5Data(digestData, digestDataLen, NULL);
	delete[] digestData;
	return result;
}

char* strDup(char const* str) {
	if (str == NULL) return NULL;
	size_t len = strlen(str) + 1;
	char* copy = new char[len];

	if (copy != NULL) {
		memcpy(copy, str, len);
	}
	return copy;
}

char* createAuthenticatorString(char const* cmd, char const* url, char const* msg, char const* username, char const* password)
{
	char realm[256]={0};
	char nonce[256]={0};
	sscanf(msg,"Digest realm=\"%[^\"]\", nonce=\"%[^\"]\"",realm,nonce);
	if (strlen(realm) != 0 && strlen(username) != 0 && strlen(password) != 0) {
		// We have a filled-in authenticator, so use it:
		char* authenticatorStr;
		if (nonce != NULL) { // Digest authentication
			char const* const authFmt =
				"Digest username=\"%s\", realm=\"%s\", "
				"nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n";
			char const* response = computeDigestResponse(cmd, realm, nonce, url, username, password);
			unsigned authBufSize = strlen(authFmt)
				+ strlen(username) + strlen(realm)
				+ strlen(nonce) + strlen(url) + strlen(response);
			authenticatorStr = new char[authBufSize];
			sprintf(authenticatorStr, authFmt,
				username, realm,
				nonce, url, response);
			free((char*)response);
		} else { // Basic authentication
			char const* const authFmt = "Authorization: Basic %s\r\n";

			unsigned usernamePasswordLength = strlen(username) + 1 + strlen(password);
			char* usernamePassword = new char[usernamePasswordLength+1];
			sprintf(usernamePassword, "%s:%s", username, password);

			char response[256] = {};
			ez_base64encode(response,usernamePassword, usernamePasswordLength);
			unsigned const authBufSize = strlen(authFmt) + strlen(response) + 1;
			authenticatorStr = new char[authBufSize];
			sprintf(authenticatorStr, authFmt, response);
			delete[] usernamePassword;
		}

		return authenticatorStr;
	}

	// We don't have a (filled-in) authenticator.
	return strDup("");
}

CCuHttpConfigClient::CCuHttpConfigClient()
{
	m_port = -1;
	m_chn_start = -1;
	m_chn_num = -1;
	m_httpReponse = NULL;
	m_bResponse = false;
	m_bDigist = false;
	//m_bDigist = true;
	m_ReSendThread =NULL;
	_connection = new TPTCPClient(this);
}

CCuHttpConfigClient::~CCuHttpConfigClient()
{
	if (m_httpReponse)
	{
		delete m_httpReponse;
		m_httpReponse = NULL;
	}

	if(_connection != NULL)
		delete _connection;

	_connection = NULL;
}

bool CCuHttpConfigClient::SetIP( const char* ip, const int port )
{
	m_ip = ip;
	m_port = port;
	return true;
}

bool CCuHttpConfigClient::SetQueryStr( const char* query_str )
{
	m_query_str = query_str;
	return true;
}

bool CCuHttpConfigClient::SetChn( const int chn_start, const int chn_num )
{
	m_chn_start = chn_start;
	m_chn_num = chn_num;
	return true;
}

bool CCuHttpConfigClient::SetJsonBody( const char* body )
{
	m_json_body = body;
	return true;
}

#include "base64/base64.h"

#ifdef OS_POSIX
#include <time.h>
unsigned long GetTickCount()
{
	struct timespec time1 = {0,0};
	clock_gettime(CLOCK_MONOTONIC, &time1);

    return time1.tv_sec*1000;
}
#endif

bool CCuHttpConfigClient::Start( std::string& output )
{
	_connection->SetSelectTimeout(3, 0);
	int ret = _connection->Connect(m_ip.c_str(), m_port);
	_connection->SetSelectTimeout(0, 0);
	if (ret <= 0 )
	{
		return false;
	}

	//!组包
	HTTPRequest http_request;
	http_request.method = M_GET;
	strcpy(http_request.host,"172.16.35.35");
	if (m_chn_num != 0 )
	{
		base::snprintf(http_request.url, sizeof(http_request.url), "%s?chstart=%d&chnum=%d", 
			m_query_str.c_str(), m_chn_start, m_chn_num);
	}
	else
	{
		base::snprintf(http_request.url, sizeof(http_request.url), "%s", m_query_str.c_str());
	}

	http_request.contentType = CONTENT_TYPE_JSON;

	if (m_json_body.length() > 0 )
	{
		http_request.setBody(m_json_body.c_str(), m_json_body.length());
	}

DIGIST:
	if(m_bDigist)
	{
		_connection->SetSelectTimeout(3, 0);
		int ret = _connection->Connect(m_ip.c_str(), m_port);
		_connection->SetSelectTimeout(0, 0);
		if (ret <= 0 )
		{
			return false;
		}

		char* authorization = createAuthenticatorString("GET",http_request.url,m_httpReponse->wwwAuthenticate, m_username.c_str(), m_passwd.c_str());
		sprintf(http_request.Authorization,  "%s", authorization);
		delete[] authorization;
	}
	else
	{
		std::string tmp,tmp1;
		tmp = m_username + ":" + m_passwd;
		base::Base64Encode(tmp,&tmp1);
		base::snprintf(http_request.Authorization, sizeof(http_request.Authorization), "Basic %s", tmp1.c_str());
	}

	char* requst_c_str = http_request.toStream();
	_connection->Send(0, requst_c_str, strlen(requst_c_str));

	unsigned long time_begin = GetTickCount();

	while(!m_bResponse)
	{
		int ret = _connection->Heartbeat();
		if ( ret < TP_NORMAL_RET )
		{
			break;
		}
		else if(ret > TP_NORMAL_RET )
		{
			if (GetTickCount() - time_begin >5000)
			{
				return false;
			}
#ifdef WIN32
			Sleep(10);
#else
            usleep(10000);
#endif
		}
	}
	_connection->Close();

	if (m_httpReponse)
	{
		if (m_httpReponse->result == 200)
		//!分析应答包
		//if (m_chn_num != 0 )
		{
			//!查询配置包,要把查询到的配置导出来
			output = m_httpReponse->getBody();
		}
		else if (m_httpReponse->result == 401 && m_bDigist== false)
		{// 用digist再来一遍

			m_bDigist = true;
			m_bResponse = false;
			goto DIGIST;
		}
		return true;
	}

	return false;
}

bool CCuHttpConfigClient::StartHeartBeat( std::string& output)
{
	DWORD dwTick = GetTickCount();
	_connection->SetSelectTimeout(1, 0);
	int ret = _connection->Connect(m_ip.c_str(), m_port);
	_connection->SetSelectTimeout(0, 0);
	DWORD dwSpan = GetTickCount() - dwTick;
	if (ret <= 0 )
	{
		return false;
	}

	//!组包
	HTTPRequest http_request;
	http_request.method = M_POST;
	if (m_chn_num != 0 )
	{
		sprintf(http_request.url,  "%s?chstart=%d&chnum=%d", 
			m_query_str.c_str(), m_chn_start, m_chn_num);
	}
	else
	{
		sprintf(http_request.url,  "%s", m_query_str.c_str());
	}

	http_request.contentType = CONTENT_TYPE_JSON;

	if (m_json_body.length() > 0 )
	{
		http_request.setBody(m_json_body.c_str(), m_json_body.length());
	}

	//std::string tmp,tmp1;
	//tmp = m_username + ":" + m_passwd;
	//base::Base64Encode(tmp,&tmp1);
	//base::snprintf(http_request.Authorization, sizeof(http_request.Authorization), "Basic %s", tmp1.c_str());
	sprintf(http_request.host,  "%s:%d", m_ip.c_str(), m_port);
	sprintf(http_request.connection, "%s", "keep-alive");

	char* requst_c_str = http_request.toStream();
	_connection->Send(0, requst_c_str, strlen(requst_c_str));

	unsigned long time_begin = GetTickCount();

	while(!m_bResponse)
	{
		int ret = _connection->Heartbeat();
		if ( ret < TP_NORMAL_RET )
		{
			break;
		}
		else if(ret > TP_NORMAL_RET )
		{
			if (GetTickCount() - time_begin > HTTPRESPONCE_TIME)
			{
				return false;
			}
#ifdef WIN32
			Sleep(1);
#else
			usleep(10000);
#endif
		}
	}
	_connection->Close();

	if (m_httpReponse && m_httpReponse->result == 200 )
	{
		//!分析应答包
		//if (m_chn_num != 0 )
		{
			//!查询配置包,要把查询到的配置导出来
			output = m_httpReponse->getBody();
		}
		return true;
	}

	return false;
}


int CCuHttpConfigClient::onData( int engineId, int connId, const char* data, int len )
{
	m_recv_buf.append(data, len);
	int readlen = 0;

	if (m_httpReponse)
	{
		delete m_httpReponse;
		m_httpReponse = NULL;
	}
	HTTPCommon* pComm = HTTPFactory::createPDUFromStream((char*)m_recv_buf.c_str(), m_recv_buf.length(), readlen, 1);
	if (pComm == NULL)
	{
		return 0;
	}
	if (pComm->getType() ==  MODEL_RESPONSE)
	{
		m_httpReponse = dynamic_cast<HTTPResponse*>(pComm);
		m_bResponse = true;
	}
	else
	{
		delete pComm;
		assert(0);
	}
	return 0;
}

int CCuHttpConfigClient::onClose( int engineId, int connId )
{
	return 0;
}

int CCuHttpConfigClient::onConnect( int engineId, int connId, const char* ip, int port )
{
	return 0;
}

int CCuHttpConfigClient::onSendDataAck( int engineId, int id, int nSeq, int sendLen )
{
	return 0;
}

int CCuHttpConfigClient::onSendStatus( int engineId, int connId, int statusType, int param )
{
	return 0;
}

int CCuHttpConfigClient::onTimeout( int id, int context )
{
	return 0;
}

bool CCuHttpConfigClient::SetUser( const char* name, const char* passwd )
{
	m_username = name;
	m_passwd = passwd;
	return true;
}
//"%s:%s@%s:%d/axis-media/media.amp?videocodec=h264&camera=%d"
bool CCuHttpConfigClient::StartCGI(std::string& url)
{
	std::string user,pass,ip,port,query;

	int nUserEnd = url.find(':');
	int nPassEnd = url.find('@',nUserEnd);
	int nIpEnd = url.find(':',nPassEnd);
	int nPortEnd = url.find('/',nIpEnd);

	user.assign(url,0,nUserEnd);
	pass.assign(url,nUserEnd+1,nPassEnd-nUserEnd-1);
	ip.assign(url,nPassEnd+1,nIpEnd-nPassEnd-1);
	port.assign(url,nIpEnd+1,nPortEnd-nIpEnd-1);
	query.assign(url,nPortEnd,url.size()-nPortEnd);

	//httpSendInfo sendInfo;
	//sendInfo.ip = ip;
	//sendInfo.user = user;
	//sendInfo.pass = pass;
	//sendInfo.query = query;
	//sendInfo.reSendTime = 0;
	//m_lockSendItem.acquire();
	//mSendItem.insert(std::make_pair(ip+query,sendInfo));
	//m_lockSendItem.release();
	//if(m_ReSendThread)
	//	return true;
	//else
	//	reSendStart();
	//return true;

	CCuHttpConfigClient httpClient;
	httpClient.SetIP(ip.c_str(), 80);
	httpClient.SetQueryStr(query.c_str());
	httpClient.SetUser(user.c_str(), pass.c_str());
	httpClient.SetChn(0, 0);

	std::string json_reponse;
	return httpClient.Start(json_reponse);
}
void CCuHttpConfigClient::reSendStart()
{
	AX_Thread::spawn(CCuHttpConfigClient::workThreadStartSend, NULL, 0, 0, &m_ReSendThread, 0, 0, 0);
}
void *CCuHttpConfigClient::workThreadStartSend(void *lpParam)
{
	//CCuHttpConfigClient* pThis = (CCuHttpConfigClient*)lpParam;
	//pThis->RunReSend();
	CCuHttpConfigClient::RunReSend();
	return 0;
}
void CCuHttpConfigClient::RunReSend()
{

	m_lockSendItem.acquire();
	//int itemSize = httpSendInfoMap.size();
	httpSendInfoMap sendMap = mSendItem;
	m_lockSendItem.release();
	while(sendMap.size()>=0)
	{
		std::vector<std::string>keySuccess,keyFailure;
		for(httpSendInfoMap::iterator iterItem = sendMap.begin();iterItem!=sendMap.end();iterItem++)
		{
			CCuHttpConfigClient httpClient;
			httpClient.SetIP(iterItem->second.ip.c_str(), 80);
			httpClient.SetQueryStr(iterItem->second.query.c_str());
			httpClient.SetUser(iterItem->second.user.c_str(), iterItem->second.pass.c_str());
			httpClient.SetChn(0, 0);
			std::string json_reponse;
			bool rect =  httpClient.Start(json_reponse);
			if(rect)
				keySuccess.push_back(iterItem->first);
			else
				keyFailure.push_back(iterItem->first);
		}
		//for delete
		m_lockSendItem.acquire();
		for(int i=0;i<keySuccess.size();i++)//success delete
		{
			httpSendInfoMap::iterator iterFind = mSendItem.find(keySuccess[i]);
			if(iterFind!=mSendItem.end())
				mSendItem.erase(iterFind);
		}
		for(int i=0;i<keyFailure.size();i++)//failure error++ if error=5 delete
		{
			httpSendInfoMap::iterator iterFind = mSendItem.find(keyFailure[i]);
			if(iterFind!=mSendItem.end())
			{
				iterFind->second.reSendTime++;
				if(iterFind->second.reSendTime>=5)
					mSendItem.erase(iterFind);
			}
		}
		sendMap = mSendItem;
		m_lockSendItem.release();
		Sleep(10);
	}
	m_ReSendThread = NULL;
}