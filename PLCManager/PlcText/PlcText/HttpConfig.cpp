#include "HttpConfig.h"

CCuHttpConfigClient::CCuHttpConfigClient()
{
	m_port = -1;
	m_chn_start = -1;
	m_chn_num = -1;
	m_httpReponse = NULL;
	m_bResponse = false;
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

//#include "base/base64.h"

#ifdef OS_POSIX
#include <time.h>
unsigned long GetTickCount()
{
	struct timespec time1 = {0,0};
	clock_gettime(CLOCK_MONOTONIC, &time1);

	return time1.tv_sec*1000;
}
#endif

bool CCuHttpConfigClient::Start( std::string& output)
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
