#pragma once

#include "GetJjhAlarmClient.h"
#include "GetJjhAlarmInfo.h"

GetJjhAlarmClient::GetJjhAlarmClient(GetJjhAlarmInfo *pParent)
:m_pParent(pParent)
,m_bConnect(false)
{
    _connection = new TPTCPClient(this);
}
GetJjhAlarmClient::~GetJjhAlarmClient()
{
	if(_connection != NULL)
	{
		delete _connection;
	    _connection = NULL;
	}
}

bool GetJjhAlarmClient::SetIP( const char* ip, const int port )
{
	m_ip = ip;
	m_port = port;
	return true;
}

bool GetJjhAlarmClient::Connect(/* std::string& output*/)
{
	if (m_bConnect)
	{
		return true;
	}
	_connection->SetSelectTimeout(1, 0);
	int ret = _connection->Connect(m_ip.c_str(), m_port);
	_connection->SetSelectTimeout(0, 0);
	if (ret <= 0 )
	{
		m_bConnect = false;
		char szlog[MAX_STR_LEN] = {0};
		sprintf_s(szlog, MAX_STR_LEN, "connect EIO-4 failed，ip: %s, port: %d", m_ip.c_str(), m_port);
		m_pParent->m_plog->TraceError(szlog);
		m_pParent->Showlog2Dlg("connect EIO-4 falied", CONNECT_PLC_ERR);

		return false;
	}
	char szlog[MAX_STR_LEN] = {0};
	sprintf_s(szlog, MAX_STR_LEN, "connect EIO-4 success，ip: %s, port: %d", m_ip.c_str(), m_port);
	m_pParent->m_plog->TraceInfo(szlog);
	m_pParent->Showlog2Dlg("connect EIO-4 success", CONNECT_PLC_SUC);
    m_bConnect = true;
	return true;
}

void GetJjhAlarmClient::SendRequest(char *pRequest, int iLength)
{
	_connection->Heartbeat();
	if (m_bConnect)
	{
		int iSendlen = _connection->Send(0, pRequest, iLength);
	}
}

int GetJjhAlarmClient::onData( int engineId, int connId, const char* data, int len)
{
    m_pParent->ParseRevData(data, len);
	return 0;
}

int GetJjhAlarmClient::onClose( int engineId, int connId )
{
	m_bConnect = false;//timeout 就重连
	m_pParent->m_plog->TraceError("disconnect EIO-4, Reconnect...");
	m_pParent->Showlog2Dlg("disconnect EIO-4, Reconnect...", CONNECT_PLC_REC);
	return 0;
}

int GetJjhAlarmClient::onConnect( int engineId, int connId, const char* ip, int port )
{
	return 0;
}

int GetJjhAlarmClient::onSendDataAck( int engineId, int id, int nSeq, int sendLen )
{
	return 0;
}

int GetJjhAlarmClient::onSendStatus( int engineId, int connId, int statusType, int param )
{
	return 0;
}

int GetJjhAlarmClient::onTimeout( int id, int context )
{
	m_bConnect = false;//timeout 就重连
	m_pParent->m_plog->TraceError("disconnect EIO-4, Reconnect...");
	m_pParent->Showlog2Dlg("disconnect EIO-4, Reconnect...", CONNECT_PLC_REC);
	return 0;
}